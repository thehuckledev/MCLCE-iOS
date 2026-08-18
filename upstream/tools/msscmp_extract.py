#!/usr/bin/env python3

import os
import sys
import struct
import subprocess
from collections import defaultdict

# helper functions
def read_c_string(data, offset):
    end = data.find(b'\x00', offset)

    if end == -1:
        return ""

    return data[offset:end].decode(
        "utf-8",
        errors="ignore"
    )


def convert_to_flac(infile, outfile):

    # skip if already exists
    if os.path.exists(outfile):
        return

    try:
        subprocess.run(
            [
                "ffmpeg",
                "-y",
                "-i", infile,
                "-c:a", "flac",
                outfile
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True
        )

        print(f"[FLAC] {outfile}")

    except Exception:
        print(f"[FAIL] ffmpeg failed on {infile}")

def main():

    if len(sys.argv) < 2:
        print("usage: python3 file_extract.py Minecraft.msscmp")
        return

    infile = sys.argv[1]

    with open(infile, "rb") as f:
        data = f.read()

    # validation
    if data[:4] != b'BANK':
        print("Not a BANK file")
        return

    filesize = len(data)

    # header recognition
    file_table_offset = struct.unpack(
        ">I",
        data[0x18:0x1C]
    )[0]

    entry_count = struct.unpack(
        ">I",
        data[0x34:0x38]
    )[0]

    print(f"[+] table @ {hex(file_table_offset)}")
    print(f"[+] entries: {entry_count}")

    if file_table_offset >= filesize:
        print("Bad table offset")
        return

    # binka and flac output folder(s)
    binka_root = "extracted_binka"
    flac_root  = "extracted_flac"

    os.makedirs(binka_root, exist_ok=True)
    os.makedirs(flac_root, exist_ok=True)

    folder_counts = defaultdict(int)
    entries_cache = []

    for i in range(entry_count):

        entry_off = file_table_offset + (i * 8)

        if entry_off + 8 > filesize:
            break

        try:

            folder_off = struct.unpack(
                ">I",
                data[entry_off:entry_off+4]
            )[0]

            info_off = struct.unpack(
                ">I",
                data[entry_off+4:entry_off+8]
            )[0]

            if folder_off >= filesize:
                continue

            if info_off >= filesize:
                continue

            # yoink audio name from parent dir
            folder = read_c_string(
                data,
                folder_off
            )

            filename_rel = struct.unpack(
                ">I",
                data[info_off+4:info_off+8]
            )[0]

            filename_off = info_off + filename_rel

            if filename_off >= filesize:
                continue

            filename = read_c_string(
                data,
                filename_off
            )

            data_off = struct.unpack(
                "<I",
                data[info_off+8:info_off+12]
            )[0]

            sample_rate = struct.unpack(
                ">I",
                data[info_off+20:info_off+24]
            )[0]

            size = struct.unpack(
                ">I",
                data[info_off+24:info_off+28]
            )[0]

            if size <= 0:
                continue

            if data_off + size > filesize:
                continue

            clean_folder = folder.replace(
                "\\",
                "/"
            ).strip("/")

            clean_name = filename.replace(
                "*",
                ""
            ).strip()

            if not clean_name.endswith(".binka"):
                clean_name += ".binka"

            folder_counts[clean_folder] += 1

            entries_cache.append(
                (
                    clean_folder,
                    clean_name,
                    data_off,
                    size,
                    sample_rate
                )
            )

        except Exception:
            continue

    # extract + convert to flac
    extracted = 0

    for (
        clean_folder,
        clean_name,
        data_off,
        size,
        sample_rate
    ) in entries_cache:

        try:

            binka_folder = os.path.join(
                binka_root,
                clean_folder
            )

            os.makedirs(
                binka_folder,
                exist_ok=True
            )

            binka_path = os.path.join(
                binka_folder,
                clean_name
            )

            with open(binka_path, "wb") as out:
                out.write(
                    data[data_off:data_off+size]
                )

            # folders with one sound get deleted
            if folder_counts[clean_folder] == 1:

                folder_parts = clean_folder.split("/")

                parent_folder = os.path.join(
                    flac_root,
                    *folder_parts[:-1]
                )

                os.makedirs(
                    parent_folder,
                    exist_ok=True
                )

                flac_filename = (
                    folder_parts[-1] + ".flac"
                )

                flac_path = os.path.join(
                    parent_folder,
                    flac_filename
                )

            else:

                flac_folder = os.path.join(
                    flac_root,
                    clean_folder
                )

                os.makedirs(
                    flac_folder,
                    exist_ok=True
                )

                flac_filename = (
                    os.path.splitext(clean_name)[0]
                    + ".flac"
                )

                flac_path = os.path.join(
                    flac_folder,
                    flac_filename
                )

            convert_to_flac(
                binka_path,
                flac_path
            )

            print(
                f"[+] {clean_name} "
                f"({size} bytes @ {sample_rate}hz)"
            )

            extracted += 1

        except Exception:
            continue

    print(f"\nDone. Extracted {extracted} files.")


if __name__ == "__main__":
    main()
