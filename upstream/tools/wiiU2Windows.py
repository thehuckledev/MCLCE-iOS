#!/usr/bin/env python3

import argparse
import os
import sys
import zlib
from collections import defaultdict

# good luck processing this
# main arguments:
# --dlc: xbox one split save stuff (meant for our current dlc system)
# --no-split: disable dlc split save system (when --dlc is present)
# otherwise the converter defaults to regular, non-split windows saves

FILE_ENTRY_SIZE = 144
SAVE_HEADER_SIZE = 12
WINDOWS_SAVE_VERSION = 9
REGION_FORMAT_16_NAME = "region_format_16"
REGION_SECTOR_BYTES = 4096
REGION_TABLE_COUNT = 1024
REGION_TABLE_BYTES = REGION_TABLE_COUNT * 4
REGION_HEADER_BYTES = REGION_TABLE_BYTES * 2


def read_u16(data, offset, endian):
    return int.from_bytes(data[offset:offset + 2], endian)


def read_u32(data, offset, endian):
    return int.from_bytes(data[offset:offset + 4], endian)


def read_u64(data, offset, endian):
    return int.from_bytes(data[offset:offset + 8], endian)


def write_u16(buf, offset, value, endian):
    buf[offset:offset + 2] = int(value).to_bytes(2, endian, signed=False)


def write_u32(buf, offset, value, endian):
    buf[offset:offset + 4] = int(value).to_bytes(4, endian, signed=False)


def write_u64(buf, offset, value, endian):
    buf[offset:offset + 8] = int(value).to_bytes(8, endian, signed=False)


def region_coords_from_name(name):
    base_name = os.path.basename(name)
    if not base_name.lower().endswith(".mcr"):
        return None

    stem = base_name[:-4]
    if stem.startswith("r."):
        parts = stem.split(".")
        if len(parts) == 3:
            try:
                return "", int(parts[1]), int(parts[2])
            except ValueError:
                return None

    if stem.startswith("DIM-1r."):
        coords = stem[len("DIM-1r."):].split(".")
        if len(coords) == 2:
            try:
                return "DIM-1", int(coords[0]), int(coords[1])
            except ValueError:
                return None

    if stem.startswith("DIM1/r."):
        coords = stem[len("DIM1/r."):].split(".")
        if len(coords) == 2:
            try:
                return "DIM1/", int(coords[0]), int(coords[1])
            except ValueError:
                return None

    return None


def make_region_name(prefix, region_x, region_z):
    if prefix == "DIM-1":
        return f"DIM-1r.{region_x}.{region_z}.mcr"
    if prefix == "DIM1/":
        return f"DIM1/r.{region_x}.{region_z}.mcr"
    return f"r.{region_x}.{region_z}.mcr"


def parse_region_chunks(region_data, endian):
    if len(region_data) < REGION_HEADER_BYTES:
        return []

    chunks = []
    for index in range(REGION_TABLE_COUNT):
        offset = read_u32(region_data, index * 4, endian)
        if offset == 0:
            continue

        sector_start = offset >> 8
        sector_count = offset & 0xFF
        if sector_start == 0 or sector_count == 0:
            continue

        chunk_offset = sector_start * REGION_SECTOR_BYTES
        if chunk_offset + 8 > len(region_data):
            continue

        stored_length = read_u32(region_data, chunk_offset, endian)
        decomp_length = read_u32(region_data, chunk_offset + 4, endian)
        use_rle = bool(stored_length & 0x80000000)
        payload_length = stored_length & 0x7FFFFFFF
        data_start = chunk_offset + 8
        data_end = data_start + payload_length
        if data_end > len(region_data):
            continue

        local_x = index & 31
        local_z = index >> 5
        chunks.append(
            {
                "local_x": local_x,
                "local_z": local_z,
                "stored_length": stored_length,
                "decomp_length": decomp_length,
                "use_rle": use_rle,
                "data": region_data[data_start:data_end],
            }
        )

    return chunks


def build_region_file(chunks, endian):
    sector_free = [True] * 4096
    sector_free[0] = False
    sector_free[1] = False

    chunks_by_slot = {}
    for chunk in chunks:
        slot = chunk["local_x"] + (chunk["local_z"] * 32)
        chunks_by_slot[slot] = chunk

    output = bytearray(REGION_SECTOR_BYTES * 2)
    header = bytearray(REGION_HEADER_BYTES)

    def allocate_sectors(byte_count):
        needed = (byte_count + REGION_SECTOR_BYTES - 1) // REGION_SECTOR_BYTES
        run_start = None
        run_count = 0
        for i in range(len(sector_free)):
            if sector_free[i]:
                if run_start is None:
                    run_start = i
                    run_count = 1
                else:
                    run_count += 1
                if run_count >= needed:
                    for j in range(run_start, run_start + needed):
                        sector_free[j] = False
                    return run_start, needed
            else:
                run_start = None
                run_count = 0
        raise ValueError("Not enough free sectors for split region file")

    for slot in sorted(chunks_by_slot.keys()):
        chunk = chunks_by_slot[slot]
        chunk_bytes = bytearray()
        chunk_bytes.extend(int(chunk["stored_length"]).to_bytes(4, endian, signed=False))
        chunk_bytes.extend(int(chunk["decomp_length"]).to_bytes(4, endian, signed=False))
        chunk_bytes.extend(chunk["data"])

        sector_start, sector_count = allocate_sectors(len(chunk_bytes))
        chunk_offset = sector_start * REGION_SECTOR_BYTES
        needed_size = chunk_offset + (sector_count * REGION_SECTOR_BYTES)
        if len(output) < needed_size:
            output.extend(b"\x00" * (needed_size - len(output)))
        output[chunk_offset:chunk_offset + len(chunk_bytes)] = chunk_bytes

        offset_value = (sector_start << 8) | sector_count
        write_u32(header, slot * 4, offset_value, endian)
        timestamp_offset = REGION_TABLE_BYTES + (slot * 4)
        write_u32(header, timestamp_offset, 0, endian)

    if len(output) < REGION_HEADER_BYTES:
        output.extend(b"\x00" * (REGION_HEADER_BYTES - len(output)))
    output[0:REGION_TABLE_BYTES] = header[0:REGION_TABLE_BYTES]
    output[REGION_TABLE_BYTES:REGION_HEADER_BYTES] = header[REGION_TABLE_BYTES:REGION_HEADER_BYTES]
    return bytes(output)


def build_split_save_entries(parsed, src_payload, src_endian, dst_endian):
    entries = []
    region_groups = defaultdict(list)

    for entry in parsed["entries"]:
        name = entry["name"]
        start = entry["start"]
        length = entry["length"]
        data = src_payload[start:start + length]

        if name == REGION_FORMAT_16_NAME:
            continue

        region_info = region_coords_from_name(name)
        if region_info is None or not name.lower().endswith(".mcr"):
            entries.append(
                {
                    "name": name,
                    "data": data,
                    "last_modified": entry["last_modified"],
                }
            )
            continue

        prefix, region_x, region_z = region_info
        region_chunks = parse_region_chunks(data, src_endian)
        for chunk in region_chunks:
            chunk_x = region_x * 32 + chunk["local_x"]
            chunk_z = region_z * 32 + chunk["local_z"]

            target_region_x = chunk_x // 16
            target_region_z = chunk_z // 16

            target_local_x = chunk_x % 16
            target_local_z = chunk_z % 16
            region_groups[(prefix, target_region_x, target_region_z)].append(
                {
                    "local_x": target_local_x,
                    "local_z": target_local_z,
                    "stored_length": chunk["stored_length"],
                    "decomp_length": chunk["decomp_length"],
                    "data": chunk["data"],
                }
            )

    entries.append(
        {
            "name": REGION_FORMAT_16_NAME,
            "data": b"",
            "last_modified": 0,
        }
    )
    entries.append(
        {
            "name": "entities.dat",
            "data": b"",
            "last_modified": 0,
        }
    )

    for prefix, region_x, region_z in sorted(region_groups.keys()):
        region_name = make_region_name(prefix, region_x, region_z)
        region_blob = build_region_file(region_groups[(prefix, region_x, region_z)], endian=dst_endian)
        entries.append(
            {
                "name": region_name,
                "data": region_blob,
                "last_modified": 0,
            }
        )

    return entries


def detect_payload_endian(payload):
    for endian in ("little", "big"):
        header_offset = read_u32(payload, 0, endian)
        header_count = read_u32(payload, 4, endian)
        if header_offset < SAVE_HEADER_SIZE:
            continue
        if header_count == 0 or header_count > 50000:
            continue
        if header_offset + (header_count * FILE_ENTRY_SIZE) == len(payload):
            return endian
    return None


def serialize_payload_from_entries(entries, original_version, target_version, dst_endian):
    data_payload = bytearray(SAVE_HEADER_SIZE)
    serialized_entries = []

    for entry in entries:
        start = len(data_payload)
        data = entry["data"]
        data_payload.extend(data)
        serialized_entries.append(
            {
                "name": entry["name"],
                "start": start,
                "length": len(data),
                "last_modified": entry.get("last_modified", 0),
            }
        )

    header_offset = len(data_payload)
    dst_payload = bytearray(data_payload)
    write_u32(dst_payload, 0, header_offset, dst_endian)
    write_u32(dst_payload, 4, len(serialized_entries), dst_endian)
    write_u16(dst_payload, 8, original_version, dst_endian)
    write_u16(dst_payload, 10, target_version, dst_endian)

    text_encoding = "utf-16-le" if dst_endian == "little" else "utf-16-be"
    header_table = bytearray()
    for entry in serialized_entries:
        record = bytearray(FILE_ENTRY_SIZE)
        name_bytes = entry["name"].encode(text_encoding, errors="ignore")
        name_bytes = name_bytes[:128]
        name_bytes = name_bytes + (b"\x00" * (128 - len(name_bytes)))
        record[0:128] = name_bytes
        write_u32(record, 128, entry["length"], dst_endian)
        write_u32(record, 132, entry["start"], dst_endian)
        write_u64(record, 136, entry["last_modified"], dst_endian)
        header_table.extend(record)

    dst_payload.extend(header_table)
    return bytes(dst_payload)


def parse_entries(payload, endian):
    header_offset = read_u32(payload, 0, endian)
    header_count = read_u32(payload, 4, endian)
    original_version = read_u16(payload, 8, endian)
    current_version = read_u16(payload, 10, endian)

    if header_offset + (header_count * FILE_ENTRY_SIZE) != len(payload):
        raise ValueError("Invalid save header layout")

    entries = []
    text_encoding = "utf-16-le" if endian == "little" else "utf-16-be"
    for i in range(header_count):
        entry_offset = header_offset + (i * FILE_ENTRY_SIZE)
        raw_name = payload[entry_offset:entry_offset + 128]
        name = raw_name.decode(text_encoding, errors="ignore").split("\x00", 1)[0]
        length = read_u32(payload, entry_offset + 128, endian)
        start = read_u32(payload, entry_offset + 132, endian)
        last_modified = read_u64(payload, entry_offset + 136, endian)
        entries.append(
            {
                "name": name,
                "length": length,
                "start": start,
                "last_modified": last_modified,
                "entry_offset": entry_offset,
            }
        )

    return {
        "header_offset": header_offset,
        "header_count": header_count,
        "original_version": original_version,
        "current_version": current_version,
        "entries": entries,
    }


def convert_region_file(region_data, src_endian, dst_endian):
    if len(region_data) < REGION_HEADER_BYTES:
        return region_data

    src = bytearray(region_data)
    dst = bytearray(region_data)

    offset_table = []
    for i in range(REGION_TABLE_COUNT):
        pos = i * 4
        value = read_u32(src, pos, src_endian)
        offset_table.append(value)
        write_u32(dst, pos, value, dst_endian)

    for i in range(REGION_TABLE_COUNT):
        pos = REGION_TABLE_BYTES + (i * 4)
        value = read_u32(src, pos, src_endian)
        write_u32(dst, pos, value, dst_endian)

    for offset_value in offset_table:
        if offset_value == 0:
            continue

        sector_start = offset_value >> 8
        sector_count = offset_value & 0xFF
        if sector_start == 0 or sector_count == 0:
            continue

        chunk_offset = sector_start * REGION_SECTOR_BYTES

        # only skip if 8 byte chunk header is unreadable
        # sector_aligned chunk limit skips the goofy chunk in the center of the world 
        if chunk_offset + 8 > len(src):
            continue

        stored_length = read_u32(src, chunk_offset, src_endian)
        decomp_length = read_u32(src, chunk_offset + 4, src_endian)

        rle_flag = stored_length & 0x80000000
        payload_length = stored_length & 0x7FFFFFFF

        write_u32(dst, chunk_offset, rle_flag | payload_length, dst_endian)
        write_u32(dst, chunk_offset + 4, decomp_length, dst_endian)

    return bytes(dst)


def serialize_payload(parsed, src_payload, src_endian, dst_endian, target_version):
    return serialize_payload_with_mode(parsed, src_payload, src_endian, dst_endian, target_version, "windows")


def serialize_payload_with_mode(parsed, src_payload, src_endian, dst_endian, target_version, mode):
    if mode == "dlc":
        split_entries = build_split_save_entries(parsed, src_payload, src_endian, dst_endian)
        return serialize_payload_from_entries(split_entries, parsed["original_version"], target_version, dst_endian)

    header_offset = parsed["header_offset"]
    entries = [entry.copy() for entry in parsed["entries"]]

    data_payload = bytearray(src_payload[:header_offset])

    for entry in entries:
        name = entry["name"].lower()
        start = entry["start"]
        length = entry["length"]

        if start + length > header_offset:
            continue
        if length == 0:
            continue

        if name.endswith(".mcr"):
            segment = src_payload[start:start + length]
            data_payload[start:start + length] = convert_region_file(segment, src_endian, dst_endian)

    dst_payload = bytearray(data_payload)
    write_u32(dst_payload, 0, header_offset, dst_endian)
    write_u32(dst_payload, 4, len(entries), dst_endian)
    write_u16(dst_payload, 8, parsed["original_version"], dst_endian)
    write_u16(dst_payload, 10, target_version, dst_endian)

    text_encoding = "utf-16-le" if dst_endian == "little" else "utf-16-be"
    header_table = bytearray()
    for entry in entries:
        record = bytearray(FILE_ENTRY_SIZE)
        name_bytes = entry["name"].encode(text_encoding, errors="ignore")
        name_bytes = name_bytes[:128]
        name_bytes = name_bytes + (b"\x00" * (128 - len(name_bytes)))
        record[0:128] = name_bytes
        write_u32(record, 128, entry["length"], dst_endian)
        write_u32(record, 132, entry["start"], dst_endian)
        write_u64(record, 136, entry["last_modified"], dst_endian)
        header_table.extend(record)

    dst_payload.extend(header_table)
    return bytes(dst_payload)


def convert_mcs(
    input_path,
    output_path,
    source_endian="auto",
    target_endian="little",
    target_version=WINDOWS_SAVE_VERSION,
    mode="windows",
    split_saves=False,
):
    with open(input_path, "rb") as f:
        blob = f.read()

    if len(blob) < 12:
        raise ValueError("Input file is too small to be a valid .mcs file")

    compressed_payload = blob[8:]
    payload = zlib.decompress(compressed_payload)

    src_endian = source_endian
    if source_endian == "auto":
        src_endian = detect_payload_endian(payload)
        if src_endian is None:
            raise ValueError("Could not detect payload endianness!")

    parsed = parse_entries(payload, src_endian)

    out_payload = serialize_payload_with_mode(parsed, payload, src_endian, target_endian, target_version, mode if split_saves else "windows")

    out_blob = bytearray()
    out_blob.extend(b"\x00\x00\x00\x00")
    out_blob.extend(len(out_payload).to_bytes(4, target_endian, signed=False))
    out_blob.extend(zlib.compress(out_payload, level=9))

    out_dir = os.path.dirname(output_path)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(output_path, "wb") as f:
        f.write(out_blob)
    out_size = len(out_blob)

    return {
        "mode": mode,
        "split_saves": split_saves,
        "source_endian": src_endian,
        "target_endian": target_endian,
        "entry_count": len(parsed["entries"]),
        "original_version": parsed["original_version"],
        "current_version": target_version,
        "decompressed_size": len(out_payload),
        "compressed_size": out_size,
    }


def default_output_path(input_path):
    input_dir = os.path.dirname(input_path)
    return os.path.join(input_dir, "saveData.ms")


def main():
    parser = argparse.ArgumentParser(description="Converts Wii U .mcs saves to Windows + Xbox One-compatible .mcs files")
    parser.add_argument("input", help="Input .mcs path")
    parser.add_argument(
        "output",
        nargs="?",
        help="Output path",
    )
    parser.add_argument(
        "--dlc",
        action="store_true",
        help="Split-save preset for DLC worlds",
    )
    parser.add_argument(
        "--no-split",
        action="store_true",
        help="Disable split-save logic when --dlc is set",
    )
    parser.add_argument(
        "--source-endian",
        choices=["auto", "big", "little"],
        default="auto",
        help="Override the input source's endianness",
    )
    parser.add_argument(
        "--target-endian",
        choices=["little", "big"],
        default="little",
        help="Override the output source's endianness",
    )
    parser.add_argument(
        "--target-version",
        type=int,
        default=WINDOWS_SAVE_VERSION,
        help="Save version to write into the file header",
    )

    args = parser.parse_args()

    try:
        mode = "dlc" if args.dlc else "windows"
        split_saves = args.dlc and not args.no_split
        output_path = args.output or default_output_path(args.input)
        result = convert_mcs(
            input_path=args.input,
            output_path=output_path,
            source_endian=args.source_endian,
            target_endian=args.target_endian,
            target_version=args.target_version,
            mode=mode,
            split_saves=split_saves,
        )
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    print("Converted save successfully!")
    print(f"  Mode: {result['mode']}")
    print(f"  DLC Split Save: {result['split_saves']}")
    print(f"  Source Endian: {result['source_endian']}")
    print(f"  Target Endian: {result['target_endian']}")
    print(f"  Entry Count: {result['entry_count']}")
    print(f"  Versions: {result['original_version']} -> {result['current_version']}")
    print(f"  Decompressed Size: {result['decompressed_size']}")
    print(f"  Output Size: {result['compressed_size']}")
    print(f"  Output Path: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())