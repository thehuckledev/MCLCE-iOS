#!/usr/bin/env python3

import argparse
import os
import sys
import zlib

FILE_ENTRY_SIZE = 144
SAVE_HEADER_SIZE = 12
REGION_FORMAT_16_NAME = "region_format_16"


def read_u16(data, offset, endian):
    return int.from_bytes(data[offset:offset + 2], endian)


def read_u32(data, offset, endian):
    return int.from_bytes(data[offset:offset + 4], endian)


def read_u64(data, offset, endian):
    return int.from_bytes(data[offset:offset + 8], endian)


def write_u32(buf, offset, value, endian):
    buf[offset:offset + 4] = int(value).to_bytes(4, endian, signed=False)


def write_u64(buf, offset, value, endian):
    buf[offset:offset + 8] = int(value).to_bytes(8, endian, signed=False)


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
            }
        )

    return {
        "header_offset": header_offset,
        "original_version": original_version,
        "current_version": current_version,
        "entries": entries,
    }


def serialize_payload_from_entries(entries, original_version, current_version, endian):
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
    out_payload = bytearray(data_payload)
    write_u32(out_payload, 0, header_offset, endian)
    write_u32(out_payload, 4, len(serialized_entries), endian)
    out_payload[8:10] = int(original_version).to_bytes(2, endian, signed=False)
    out_payload[10:12] = int(current_version).to_bytes(2, endian, signed=False)

    text_encoding = "utf-16-le" if endian == "little" else "utf-16-be"
    header_table = bytearray()
    for entry in serialized_entries:
        record = bytearray(FILE_ENTRY_SIZE)
        name_bytes = entry["name"].encode(text_encoding, errors="ignore")
        name_bytes = name_bytes[:128]
        name_bytes = name_bytes + (b"\x00" * (128 - len(name_bytes)))
        record[0:128] = name_bytes
        write_u32(record, 128, entry["length"], endian)
        write_u32(record, 132, entry["start"], endian)
        write_u64(record, 136, entry["last_modified"], endian)
        header_table.extend(record)

    out_payload.extend(header_table)
    return bytes(out_payload)


def toggle_region_format_16(input_path, output_path, source_endian="auto"):
    with open(input_path, "rb") as f:
        blob = f.read()

    if len(blob) < 12:
        raise ValueError("Input file is too small to be a valid save file")

    compressed_payload = blob[8:]
    payload = zlib.decompress(compressed_payload)

    endian = source_endian
    if endian == "auto":
        endian = detect_payload_endian(payload)
        if endian is None:
            raise ValueError("Could not detect payload endianness!")

    parsed = parse_entries(payload, endian)

    has_marker = any(e["name"] == REGION_FORMAT_16_NAME for e in parsed["entries"])

    entries_out = []
    for e in parsed["entries"]:
        if e["name"] == REGION_FORMAT_16_NAME:
            continue
        entries_out.append(
            {
                "name": e["name"],
                "data": payload[e["start"]:e["start"] + e["length"]],
                "last_modified": e["last_modified"],
            }
        )

    if has_marker:
        action = "removed"
    else:
        action = "added"
        entries_out.append(
            {
                "name": REGION_FORMAT_16_NAME,
                "data": b"",
                "last_modified": 0,
            }
        )

    out_payload = serialize_payload_from_entries(
        entries_out, parsed["original_version"], parsed["current_version"], endian
    )

    out_blob = bytearray()
    out_blob.extend(b"\x00\x00\x00\x00")
    out_blob.extend(len(out_payload).to_bytes(4, endian, signed=False))
    out_blob.extend(zlib.compress(out_payload, level=9))

    out_dir = os.path.dirname(output_path)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(output_path, "wb") as f:
        f.write(out_blob)

    return {
        "action": action,
        "endian": endian,
        "entry_count": len(entries_out),
    }


def main():
    parser = argparse.ArgumentParser(
        description="Toggles the region_format_16 marker entry in any given mcs or ms save"
    )
    parser.add_argument("input", help="Input .mcs/.ms path")
    parser.add_argument(
        "output",
        nargs="?",
        help="Output path (overwrites the input file by default)",
    )
    parser.add_argument(
        "--source-endian",
        choices=["auto", "big", "little"],
        default="auto",
        help="Override the input source's endianness",
    )

    args = parser.parse_args()
    output_path = args.output or args.input

    try:
        result = toggle_region_format_16(
            input_path=args.input,
            output_path=output_path,
            source_endian=args.source_endian,
        )
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    print(f"region_format_16 {result['action']} successfully!")
    print(f"  Endian: {result['endian']}")
    print(f"  Entry Count: {result['entry_count']}")
    print(f"  Output Path: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
