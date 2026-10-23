#!/usr/bin/env python3
"""Fail the build when the firmware image exceeds the flash or RAM budget."""

import subprocess
import sys

FLASH_LIMIT = 96 * 1024
RAM_LIMIT = 64 * 1024


def main() -> int:
    elf = sys.argv[1]
    output = subprocess.check_output(["arm-none-eabi-size", "-B", elf], text=True)
    print(output, end="")
    lines = [line for line in output.splitlines() if line.strip()]
    header = lines[0].split()
    values = lines[-1].split()
    columns = dict(zip(header, values))
    text = int(columns["text"])
    data = int(columns["data"])
    bss = int(columns["bss"])
    flash = text + data
    ram = data + bss
    print(f"flash_bytes={flash} ram_bytes={ram} limit_flash={FLASH_LIMIT} limit_ram={RAM_LIMIT}")
    if flash > FLASH_LIMIT or ram > RAM_LIMIT:
        print("size budget exceeded", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
