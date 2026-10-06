"""Build the original Civic 89 city mark using only Python's standard library.

Copyright 2026 Civic 89 contributors. SPDX-License-Identifier: GPL-3.0-or-later
No inherited or third-party image is read or modified.
"""
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1]
RECTANGLES = [(0, 0, 64, 64, "14212e"), (8, 12, 12, 44, "67d7b0"),
              (26, 24, 12, 32, "4aa1d8"), (44, 6, 12, 50, "e8c077")]
for x, y, width, height, _ in list(RECTANGLES)[1:]:
    for row in range(y + 5, y + height - 3, 8):
        RECTANGLES.append((x + 3, row, 6, 3, "14212e"))
RECTANGLES.append((7, 57, 50, 2, "ffffff"))


def png(size):
    rows = []
    for row in range(size):
        pixels = bytearray()
        for column in range(size):
            colour = "000000"
            for x, y, width, height, candidate in RECTANGLES:
                if x <= column * 64 / size < x + width and y <= row * 64 / size < y + height:
                    colour = candidate
            pixels.extend(bytes.fromhex(colour) + b"\xff")
        rows.append(b"\0" + pixels)

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)

    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(b"".join(rows))) + chunk(b"IEND", b""))


def main():
    directory = ROOT / "assets/branding"
    directory.mkdir(parents=True, exist_ok=True)
    svg = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">\n'
    svg += '<!-- Copyright 2026 Civic 89 contributors. SPDX-License-Identifier: GPL-3.0-or-later -->\n'
    svg += "".join(f'  <rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#{colour}"/>\n'
                   for x, y, w, h, colour in RECTANGLES)
    (directory / "civic89.svg").write_text(svg + "</svg>\n", encoding="utf-8")
    sizes = (16, 32, 48, 64, 256)
    images = [png(size) for size in sizes]
    offset = 6 + 16 * len(images)
    entries = []
    for size, data in zip(sizes, images):
        entries.append(struct.pack("<BBBBHHII", size % 256, size % 256, 0, 0, 1, 32, len(data), offset))
        offset += len(data)
    (directory / "civic89.ico").write_bytes(struct.pack("<HHH", 0, 1, len(images)) + b"".join(entries) + b"".join(images))
    preview = ROOT / "out/audit/m6-brand-preview.png"
    preview.parent.mkdir(parents=True, exist_ok=True)
    preview.write_bytes(images[3])


if __name__ == "__main__":
    main()
