#!/usr/bin/env python3

import struct
import argparse
import os
import sys

# default endian
ENDIAN = ">"

# defaults
DEFAULT_PCK_TYPE = 3
DEFAULT_XML_VERSION = 4
DEFAULT_ASSET_TYPE = 0


def write_u32(f, value):
    f.write(struct.pack(f"{ENDIAN}I", value))


def write_utf16_string(f, text):
    """
    PCK strings:
        uint32 length
        UTF-16 bytes
        uint32 padding
    """

    encoding = "utf-16-be" if ENDIAN == ">" else "utf-16-le"

    encoded = text.encode(encoding)

    write_u32(f, len(text))
    f.write(encoded)

    # padding
    write_u32(f, 0)


def collect_folder_assets(folder_path):

    assets = []

    for root, _, files in os.walk(folder_path):

        for file in files:

            full_path = os.path.join(root, file)

            rel_path = os.path.relpath(full_path, folder_path)

            # PCKs usually use backslashes
            rel_path = rel_path.replace("/", "\\")
            rel_path = rel_path.replace("\\\\", "\\")

            with open(full_path, "rb") as f:
                data = f.read()

            assets.append({
                "name": rel_path,
                "size": len(data),
                "type": DEFAULT_ASSET_TYPE,
                "params": {},
                "data": data,
            })

    return assets


def pack_folder_to_pck(input_folder, output_pck):

    assets = collect_folder_assets(input_folder)

    if not assets:
        raise ValueError("No files found in folder")

    # parameter lookup table
    lookup = [
        "PATH",
        "TYPE",
        "XMLVERSION",
    ]

    print(f"Assets: {len(assets)}")

    with open(output_pck, "wb") as f:

        # ----- HEADER -----

        write_u32(f, DEFAULT_PCK_TYPE)

        write_u32(f, len(lookup))

        # ----- PARAMETER LOOKUP TABLE -----

        for idx, key in enumerate(lookup):

            write_u32(f, idx)

            write_utf16_string(f, key)

        # optional XMLVERSION
        if "XMLVERSION" in lookup:
            write_u32(f, DEFAULT_XML_VERSION)

        # ----- ASSET TABLE -----

        write_u32(f, len(assets))

        for asset in assets:

            write_u32(f, asset["size"])
            write_u32(f, asset["type"])

            write_utf16_string(f, asset["name"])

            print(f"Indexing: {asset['name']} ({asset['size']} bytes)")

        # ----- ASSET DATA -----

        for asset in assets:

            params = asset["params"]

            write_u32(f, len(params))

            for key, value in params.items():

                key_index = lookup.index(key)

                write_u32(f, key_index)

                write_utf16_string(f, value)

            f.write(asset["data"])

            print(f"Writing: {asset['name']}")

    print(f"\nDone! Wrote: {output_pck}")


def main():

    global ENDIAN

    parser = argparse.ArgumentParser(
        description="Pack a folder into a Minecraft Legacy Console .pck"
    )

    parser.add_argument(
        "input",
        help="Input folder"
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Output .pck filename"
    )

    parser.add_argument(
        "--little",
        action="store_true",
        help="Write little-endian PCK"
    )

    args = parser.parse_args()

    input_path = args.input

    if not os.path.isdir(input_path):
        print(f"Input folder not found: {input_path}")
        sys.exit(1)

    if args.little:
        ENDIAN = "<"

    output_path = args.output

    if not output_path:
        output_path = os.path.basename(
            os.path.normpath(input_path)
        ) + ".pck"

    try:
        pack_folder_to_pck(input_path, output_path)

    except Exception as e:
        print(f"\nERROR: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
