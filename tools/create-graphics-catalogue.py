"""Record the complete M9 input mapping without changing any artwork.

SPDX-License-Identifier: GPL-3.0-or-later
Run with --check to verify the committed catalogue and pinned input hashes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FRAMES = (5, 9, 12, 9, 17, 3, 6)
GHOSTS = {0: "res", 1: "com", 2: "ind", 3: "fire", 5: "police",
          10: "stadium", 12: "seaport", 13: "coal", 14: "nuclear", 15: "airport"}


def catalogue():
    inventory = json.loads((ROOT / "assets/runtime-assets.json").read_text())
    pinned = {item["path"]: item for item in inventory["files"]}
    entries = []

    def add(path, **mapping):
        record = pinned[path]
        data = (ROOT / path).read_bytes()
        if record["hash_mode"] == "lf":
            data = data.replace(b"\r\n", b"\n")
        if hashlib.sha256(data).hexdigest() != record["sha256"]:
            raise ValueError(f"Input differs from pinned inventory: {path}")
        dimensions = re.search(rb'"(\d+) (\d+) \d+ \d+"', data)
        if not dimensions:
            raise ValueError(f"Missing XPM dimensions: {path}")
        entries.append({key: record[key] for key in
                        ("path", "sha256", "hash_mode", "licence_group", "source")} |
                       {"width": int(dimensions[1]), "height": int(dimensions[2])} | mapping)

    add("images/tiles.xpm", kind="tiles", ids=[0, 959], source_cell=[16, 16],
        source_rectangle="0,id*16,16,16", destination_rectangle="(id%32)*16*d,(id/32)*16*d,16*d,16*d")
    add("images/tilessm.xpm", kind="minimap", ids=[0, 959], source_cell=[3, 3],
        source_rectangle="0,id*3,3,3", destination_rectangle="0,id*3*d,3*d,3*d")
    for sprite, frames in enumerate(FRAMES, 1):
        for frame in range(frames):
            add(f"images/obj{sprite}-{frame}.xpm", kind="sprite", sprite=sprite, frame=frame)
    for tool, name in GHOSTS.items():
        add(f"images/{name}.xpm", kind="preview", tool=tool)
    return {"schema": 1, "baseline": inventory["baseline"], "tile_count": 960,
            "sprite_frames": sum(FRAMES), "textured_previews": len(GHOSTS),
            "density": {"Classic": 1, "Enhanced": 2},
            "transform": "src/GraphicsArt.cpp: refinePixelRegion (per-region clamped neighbours)",
            "retained": ["interface icons", "overlay colours/meaning", "world geometry", "animation cadence"],
            "rights": "Inherited GPL/additional terms; per-asset public-release clearance remains pending.",
            "inputs": entries}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    target = ROOT / "assets/graphics-catalogue.json"
    result = catalogue()
    if args.check:
        if json.loads(target.read_text()) != result:
            raise SystemExit("Graphics catalogue differs; regenerate and review it.")
        print("73 graphics inputs, pinned hashes and complete mappings verified.")
    else:
        target.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8", newline="\n")
