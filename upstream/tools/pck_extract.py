#!/usr/bin/env python3

import struct
import argparse
import zipfile
import os
import sys

# default endian mode
ENDIAN = ">"

def detect_endianness(f):
    """
    Detect whether the PCK uses big-endian or little-endian.
    """

    global ENDIAN

    pos = f.tell()

    raw = f.read(4)

    if len(raw) != 4:
        raise EOFError("File too small")

    be = struct.unpack(">I", raw)[0]
    le = struct.unpack("<I", raw)[0]

    # detect endianness based off of version numbers [usually 3 or 4]
    if 0 < be < 100:
        ENDIAN = ">"
    elif 0 < le < 100:
        ENDIAN = "<"
    else:
        raise ValueError("Could not determine endianness")

    f.seek(pos)

    print(f"Detected {'Big' if ENDIAN == '>' else 'Little'} Endian")


def read_u32(f):
    data = f.read(4)

    if len(data) != 4:
        raise EOFError("Unexpected EOF while reading uint32")

    return struct.unpack(f"{ENDIAN}I", data)[0]


def read_utf16_string(f):
    """
    PCK strings are:
        uint32 length
        UTF-16 bytes
        uint32 padding
    """

    length = read_u32(f)

    if length > 100000:
        raise ValueError(f"Unreasonable string length: {length}")

    raw = f.read(length * 2)

    if len(raw) != length * 2:
        raise EOFError("Unexpected EOF while reading string")

    encoding = "utf-16-be" if ENDIAN == ">" else "utf-16-le"

    text = raw.decode(encoding, errors="replace")

    # skip padding
    padding = f.read(4)

    if len(padding) != 4:
        raise EOFError("Unexpected EOF while reading string padding")

    return text


def extract_pck_to_zip(input_file, output_zip):

    with open(input_file, "rb") as f:

        # detect endianness before reading anything
        detect_endianness(f)

        # ----- HEADER -----

        pck_type = read_u32(f)
        param_count = read_u32(f)

        print(f"PCK Type: {pck_type}")
        print(f"Parameter Count: {param_count}")

        # ----- PARAMETER LOOKUP TABLE -----

        lookup = [None] * param_count

        for _ in range(param_count):

            idx = read_u32(f)
            key = read_utf16_string(f)

            if idx >= param_count:
                raise ValueError(f"Invalid parameter index: {idx}")

            lookup[idx] = key

        # Optional XMLVERSION field

        if "XMLVERSION" in lookup:
            xml_version = read_u32(f)
            print(f"XML Version: {xml_version}")

        # ----- ASSET TABLE -----

        asset_count = read_u32(f)

        print(f"Asset Count: {asset_count}")

        assets = []

        for i in range(asset_count):

            size = read_u32(f)
            asset_type = read_u32(f)
            name = read_utf16_string(f)

            name = name.replace("\\", "/")

            print(f"[{i+1}/{asset_count}] {name} ({size} bytes)")

            assets.append({
                "name": name,
                "size": size,
                "type": asset_type,
            })

        # ----- ASSET DATA -----

        for asset in assets:

            asset_param_count = read_u32(f)

            params = {}

            for _ in range(asset_param_count):

                key_index = read_u32(f)
                value = read_utf16_string(f)

                if key_index < len(lookup):
                    key = lookup[key_index]
                    params[key] = value

            asset["params"] = params

            data = f.read(asset["size"])

            if len(data) != asset["size"]:
                raise EOFError(
                    f"Unexpected EOF while reading asset data: {asset['name']}"
                )

            asset["data"] = data

        # ----- WRITE ZIP -----

        print(f"\nWriting ZIP: {output_zip}")

        with zipfile.ZipFile(
            output_zip,
            "w",
            compression=zipfile.ZIP_DEFLATED
        ) as zf:

            for asset in assets:

                zip_name = asset["name"].lstrip("/")

                if not zip_name:
                    continue

                print(f"Adding: {zip_name}")

                zf.writestr(zip_name, asset["data"])

    print("\nDone!")


def main():

    parser = argparse.ArgumentParser(
        description="Convert Minecraft Legacy Console .pck files to .zip"
    )

    parser.add_argument(
        "input",
        help="Input .pck file"
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Output zip filename"
    )

    args = parser.parse_args()

    input_path = args.input

    if not os.path.isfile(input_path):
        print(f"Input file not found: {input_path}")
        sys.exit(1)

    output_path = args.output

    if not output_path:
        output_path = os.path.splitext(input_path)[0] + ".zip"

    try:
        extract_pck_to_zip(input_path, output_path)

    except Exception as e:
        print(f"\nERROR: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()