"""Refresh the reviewed runtime inventory; never run automatically during a build."""

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASELINE = "9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad"
FONT_COMMIT = "6c67ab1f7aa65c442bd2745bb9d4ef1cd7bc01fa"
FONT_BASE = f"https://raw.githubusercontent.com/impallari/Raleway/{FONT_COMMIT}"
FONTS = {
    "res/Raleway-Medium.ttf": "fonts/v3.000%20Fontlab/TTF/Raleway-Medium.ttf",
    "res/Raleway-Bold.ttf": "fonts/v3.000%20Fontlab/TTF/Raleway-Bold.ttf",
    "res/Raleway-BoldItalic.ttf": "fonts/v3.000%20Fontlab/TTF/Raleway-Bold-Italic.ttf",
    "res/virtue.ttf": "fonts/v3.000%20Fontlab/TTF/Raleway-Bold.ttf",
    "res/licenses/raleway/OFL.txt": "OFL.txt",
}


def main():
    files = set(FONTS)
    references = {}
    for source in sorted((ROOT / "src").rglob("*")):
        if source.suffix not in {".cpp", ".h"}:
            continue
        data = source.read_bytes()
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            text = data.decode("cp1252")  # Some inherited copyright headers use Windows-1252.
        for match in re.finditer(r'"((?:res|images|icons)/[^"\n]+\.(?:ttf|png|xpm|json))"', text):
            path = match.group(1)
            # Windows is case insensitive; retain exact spellings as audit evidence.
            canonical = next((font for font in FONTS if font.lower() == path.lower()), path)
            files.add(canonical)
            references.setdefault(canonical, set()).add(source.relative_to(ROOT).as_posix())
    # Sprite.cpp constructs these paths at runtime. Counts match initSprite exactly.
    for sprite, frames in {1: 5, 2: 9, 3: 12, 4: 9, 5: 17, 6: 3, 7: 6}.items():
        for frame in range(frames):
            path = f"images/obj{sprite}-{frame}.xpm"
            files.add(path)
            references[path] = {"src/Sprite.cpp"}
    files.add("icons/LICENSE.txt")
    files.update(path.relative_to(ROOT).as_posix() for path in (ROOT / "scenarios").glob("snro.*"))
    files.update(path.relative_to(ROOT).as_posix() for path in (ROOT / "cities").glob("*.cty"))
    records = []
    for path in sorted(files):
        data = (ROOT / path).read_bytes()
        hash_mode = "lf" if Path(path).suffix in {".xpm", ".json", ".txt"} else "raw"
        if hash_mode == "lf":
            data = data.replace(b"\r\n", b"\n")
        records.append({
            "path": path,
            "sha256": hashlib.sha256(data).hexdigest(),
            "bytes": len(data),
            "hash_mode": hash_mode,
            "licence_group": "raleway" if path in FONTS else "inherited-icons" if path.startswith("icons/") else "inherited-sdlpp",
            "source": f"{FONT_BASE}/{FONTS[path]}" if path in FONTS else f"https://github.com/ldicker83/Micropolis-SDLPP/blob/{BASELINE}/{path}",
            "referenced_by": sorted(references.get(path, {"src/FileIo.cpp"} if path.startswith(("cities/", "scenarios/")) else set())),
        })
    embedded = []
    for path in ["Micropolis.png", "micropolis.ico"]:
        data = (ROOT / path).read_bytes()
        embedded.append({
            "path": path, "sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data),
            "hash_mode": "raw", "licence_group": "inherited-embedded",
            "source": f"https://github.com/ldicker83/Micropolis-SDLPP/blob/{BASELINE}/{path}",
            "referenced_by": ["micropolis-sdlpp.rc"],
        })
    path = "assets/branding/civic89.ico"
    data = (ROOT / path).read_bytes()
    embedded.append({
        "path": path, "sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data),
        "hash_mode": "raw", "licence_group": "civic89-branding",
        "source": "Original Civic 89 source: tools/create-branding-icon.py and assets/branding/civic89.svg",
        "referenced_by": ["packaging/civic89.rc.in"],
    })
    inventory = {"baseline": BASELINE, "font_commit": FONT_COMMIT, "files": records, "embedded_files": embedded}
    (ROOT / "assets" / "runtime-assets.json").write_text(json.dumps(inventory, indent=2) + "\n", encoding="utf-8")
    print(f"Inventoried {len(records)} files; review hashes and licence records before committing.")


if __name__ == "__main__":
    main()
