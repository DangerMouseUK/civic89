# ADR 0007 — M6 release engineering

> Historical delivery decision. Current faithful scope is ADR 0009;
> [ADR 0010](0010_PUBLIC_BETA.md) supersedes blanket public-download blockers
> for the owner-approved unsigned beta, without passing physical/asset/signing gates.

Date: 6 October 2026. Scope: all M6-01 through M6-05 plus the root README,
on `codex/m6-release-engineering`, based on merged M5 `8db4b1e`.

## Decisions

1. Ship development tooling under semantic version `0.6.0-dev`. Windows resources,
   CLI/log identity and JSON provenance share version, commit and architecture.
   Official packaging requires clean committed inputs and a matching reconfigured
   Release build. Source ZIPs include an explicit export identity; archive builds
   identify modifications as untracked instead of inheriting parent Git state.
2. Produce a complete portable ZIP, per-user Inno Setup installer, matching source
   ZIP and SHA-256 inventory. Reuse the pinned vcpkg graph and all 122 staged assets.
   Bundle only selected-compiler release CRT files plus their separate notices;
   inspect static imports and all packaged PE architectures. No production dependency added.
3. Default delivery is unsigned development artifacts. Public mode fails on the
   committed rights/brand/physical acceptance gates and requires trusted signing.
   A separate manually dispatched main-only protected signing workflow authenticates
   successful main CI artifacts before using a provisioned certificate-backed key.
   No PR job holds signing material; no keys, public release or version tags are created.
4. Windows installer is per user, no elevation, preserves user data and removes
   managed files without recursive deletion. The portable update strategy uses
   isolated whole-version directories, verified offline archives, an exclusive lock,
   pre-publication smoke, flushed atomic current-pointer replacement and rollback.
   Keep earlier/orphaned directories for inspection. Installer upgrades instead use
   Restart Manager and a retained earlier installer for manual rollback; they are
   not presented as whole-directory transactions.
5. Automatic network updates wait for stable versioning, approved release gates
   and GitHub Releases hosting, as the roadmap requires. The tested offline helper
   accepts an explicit trusted checksum; it is not an authenticated update service.
   Power-loss/filesystem-corruption guarantees are outside process-crash acceptance.
6. Native ARM64 Release uses the same sources, assets and dependency baseline,
   `arm64-windows` triplets and a native hosted runner for build/test/package/install
   acceptance. x64 remains primary; existing x64 sanitizer configurations remain.
7. Replace the CMake executable's inherited product resource with the original
   Civic 89 geometric city icon and version info. Retain the complete inherited
   `.rc`/Visual Studio comparison project and asset files. The upstream README is
   preserved under `project/reference/README_SDLPP.md`; the root is now Civic 89's
   accurate landing/build/compatibility document.

## Classic and data compatibility

No engine algorithm, RNG, scenario fixture, tool cost/rule, golden digest, source
location or city-file layout changes. User data keeps `%APPDATA%\Civic89\Civic89`.
Production asset lookup now prefers the executable directory, so shortcuts and
portable launches work from another cwd. The retained comparison can still use
repository-relative assets. `--version` is non-mutating; `--smoke-test` uses a
unique temporary user directory and never restores/writes actual user recovery.

## Gates and acceptance

See [M6 evidence](../../tests/baseline/M6_2026-10-06.md) for exact checks/results.
Signing integration does not establish a trusted public signer: no identity/host
is provisioned here. Asset rights, formal brand review and physical clean-machine
desktop/audio/dialog/mixed-DPI acceptance remain explicit release gates. Historical
27,120-byte save compatibility and exact scenario/RNG restoration are unchanged
limitations. These gates prevent public publication, not private engineering
candidate generation. The following milestone must not silently claim a stable
Classic contract until those acceptance decisions are resolved.
