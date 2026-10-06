# Civic 89 release engineering

## Version, compatibility and publication boundary

M6 supplies the complete packaging/signing/update/ARM64 engineering path; M7 adds
mode-aware save acceptance to that delivery.
`0.8.0-dev` identifies development candidates; it is not a stable Classic release
or a city-format version. The current 51,360-byte save layout and all Classic
goldens are retained. Older 27,120-byte saves and exact RNG/scenario replay remain
unsupported. Enhanced v1 stores the same ordinary-city payload in a versioned
`.c89` container. See [Classic compatibility](CLASSIC_COMPATIBILITY.md),
[Enhanced format](ENHANCED_CITY_FORMAT.md) and [BUILDING.md](BUILDING.md).

Update the CMake project version, `vcpkg.json`, `src/BuildInfo.h` comparison fallback,
README and CHANGELOG together for a version change. CMake records full commit,
dirty state, compiler, architecture and pinned dependency baseline in `build-info.json`;
`--version`, startup logs and Windows executable properties expose the identity.
Commit and reconfigure release inputs before packaging. A dirty/stale build fails.

Public release is blocked until the explicit `packaging/release-gates.json` records
are resolved. Inherited icon/per-asset rights, brand review and physical desktop
acceptance are still pending. Public mode also requires a trusted signing identity.
CI produces unsigned **development candidates** and does not create tags or public releases.

## Download and play on another machine

For M8 personal testing, use the repository's **draft** playtest release while
signed into an account with repository write access. Choose `windows-x64.zip` for
Intel/AMD Windows 11 or `windows-arm64.zip` for ARM Windows 11. Extract the entire
ZIP into a folder and double-click `civic89.exe`. Keep all included DLLs and asset
folders together. No compiler, vcpkg, SDL installation or PowerShell is needed to
play. The architecture's `setup.exe` is an alternative per-user installer.

Normal launch keeps application settings/recovery in the user profile. For an
isolated desktop acceptance session, run `civic89.exe --desktop-test`; it starts
Detroit with temporary settings/recovery. Use copies of current saves. The
[plain-English test instructions](../packaging/DEVELOPMENT_RELEASE_NOTES.md)
describe the checks to try. A successful download/build does not complete physical
M8 acceptance or clear public asset/brand/signing gates.

## Repeatable development drafts

The manually dispatched **release-draft** workflow takes a successful
`windows-ci` run ID and a fresh `playtest-*` tag. It becomes available after this
workflow is merged into `main`; it runs trusted main scripts, accepts repository-owned
CI runs (including an unmerged PR candidate), authenticates the actual build
commit and downloads both architectures. PR CI builds use an integration merge;
the workflow checks that its second parent is the reviewed run's head SHA.
It does not require merging M8 to stage its existing green build locally.

`New-DevelopmentRelease.ps1` verifies all ZIP/installer/source checksums, portable
inventory/PE/build identity, clean build state and corresponding-source identity.
It checks every source member before sharing one source ZIP between architectures
(runner ZIP headers/compression can differ). It combines checksums and names
delivery metadata by architecture so the files do not overwrite each other.
Downloaded code is never executed by the release job. CI tests this verification
and rejection of wrong commits, tampered installers, missing hashes and wrong
source identity even when its outer checksum is valid.

For a local green CI candidate, authenticate its run and download both artifacts:

```powershell
$run = '<successful windows-ci run ID>'
gh run view $run --repo DangerMouseUK/civic89
gh run download $run --repo DangerMouseUK/civic89 --name candidate-x64 --dir out/releases/input-x64
gh run download $run --repo DangerMouseUK/civic89 --name candidate-arm64 --dir out/releases/input-arm64
./tools/release/New-DevelopmentRelease.ps1 `
  -DeliveryDirectory @('out/releases/input-x64','out/releases/input-arm64') `
  -ExpectedCommit '<full actual delivery commit from the authenticated CI run>' `
  -Tag playtest-m8-1 -NotesFile packaging/DEVELOPMENT_RELEASE_NOTES.md `
  -OutputDirectory out/releases/playtest-m8-1
```

Use `-VerifyOnly` to stage/inspect assets without any GitHub write. Output must be
fresh. Without that switch, the script creates a lightweight tag at the immutable
build commit and a **draft prerelease**, explicitly excluded from Latest. It never
moves an existing tag, replaces a release/asset or publishes. Reusing a release
tag fails; use a new numbered tag for the next candidate. A draft is visible only
to repository writers; it is suitable for the owner's other machine, not a public
download. Public prereleases still require the public-release checklist below.

The workflow needs only `contents: write` and `actions: read` for its explicit
draft job. Normal CI retains read-only access and cannot publish a release or
access signing credentials. Stable, signed public releases continue through the
separate signing stage and final reviewed publication below.

## Build a complete x64 delivery

Use PowerShell **7**, CMake 4.2+, the VS 2026 toolchain and pinned external vcpkg.
Starting from a clean committed checkout with CMake on PATH:

```powershell
$env:VCPKG_ROOT = 'C:\Dev\vcpkg'
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release -- /m
ctest --preset windows-x64-release --output-on-failure
./tools/release/Bootstrap-InnoSetup.ps1 -Destination "$env:TEMP\civic89-inno-6.7.3"
./tools/release/Package-Release.ps1 -Iscc "$env:TEMP\civic89-inno-6.7.3\ISCC.exe"
```

The compiler bootstrap pins Inno Setup 6.7.3 by SHA-256 and verifies its publisher
signature before installing the build tool at the requested location. Use a fresh
destination, or reuse that verified `ISCC.exe` without re-running bootstrap.
The pinned download comes from the [official release](https://github.com/jrsoftware/issrc/releases/tag/is-6_7_3).
Inno Setup is build tooling, not a game dependency.

`out/releases/civic89-0.8.0-dev-windows-x64/` contains:

- Portable ZIP and per-user installer EXE.
- Matching source ZIP from the exact build commit, with `source-provenance.json`
  so an extracted source build does not require `.git` or inherit a parent's identity.
- `SHA256SUMS.txt` for those three deliverables and `delivery.json` with identity.
- Inspectable staging/extracted verification trees retained locally.

The ZIP includes the executable, pinned dependency DLLs, release CRT DLLs from the
selected compiler's redistributable directory, inventoried assets/fonts, licences,
notices, build identity and a per-file SHA-256 manifest. Tests, runner binaries,
PDBs, debug/ASan DLLs and user preferences/recovery are excluded. Dependency copyright
files cover the installed pinned ports, including transitive libraries. Microsoft
CRT distribution retains its separate terms and `Redist.txt`; see
[Microsoft's documentation](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files).
Windows 11 supplies the Universal CRT. App-local CRT servicing needs a new package.
The ARM64 CRT directory includes an optional x64-base ARM64X exception runtime.
Only native-header CRT files are copied; static import closure rejects any omitted
DLL that the package requires. The strict native-architecture check remains in force.
See [Microsoft's ARM64X description](https://learn.microsoft.com/en-us/windows/arm/arm64x-pe).
Extracted source builds display an untracked-modifications annotation because no
Git working-tree comparison is available. Official packaging requires a clean Git checkout.

Verification launches both staging and extracted ZIPs with developer DLL paths
removed and an unrelated working directory. `--smoke-test` uses dummy SDL drivers
and a unique temporary user-data directory: it loads Detroit, renders the modern
UI, saves/reloads Classic, explicitly imports Enhanced, saves/reloads its tagged
container, exports a Classic copy and tears down. It does not modify real preferences or saves.
This is not a substitute for clean-machine visible/audio/DPI acceptance.

```powershell
$delivery = 'out/releases/civic89-0.8.0-dev-windows-x64'
./tools/release/Test-Release.ps1 -Directory "$delivery/portable"
./tools/release/Test-ReleaseTransactions.ps1 -DeliveryDirectory $delivery
./tools/release/Test-Installer.ps1 -DeliveryDirectory $delivery
```

Installer acceptance requires no existing Civic 89 installer registration. It
tests a dedicated directory, install/reinstall, actual installed launch, uninstall
and retention of user-created Classic and Enhanced city files. It never adopts a user's installation.

## Installer behaviour

The installer requires Windows 11, installs per user without elevation under
`%LOCALAPPDATA%\Programs\Civic 89\<architecture>`, offers a Start Menu shortcut
and optional desktop shortcut, and registers an uninstaller. It closes the running
application through Windows Restart Manager before replacing application files.
It does not install file associations, migrate save formats, recursively delete
the installation directory or remove `%APPDATA%\Civic89\Civic89`.

Upgrade requires closing/saving the game first. User-selected saves, preferences
and ordinary-city recovery are outside the managed package. For crash rollback
of an installer upgrade, keep the previous installer and reinstall it after
closing the game; Inno installer replacement is not claimed to be a transactional
whole-directory update. Use the portable strategy below when that property is required.

## Separate secure signing stage

Unsigned CI has no signing credentials. The manually dispatched `release-signing`
workflow is restricted to `main`, a protected `release-signing` environment and
dedicated self-hosted Windows signing runners labelled `civic89-signing` plus
their native `X64`/`ARM64` architecture. Provision approval protection on that
environment and a certificate-backed private key in CurrentUser/My on the runner.
Set its `SIGNING_THUMBPRINT`, `SIGNTOOL_PATH` and `INNO_ISCC_PATH` variables. Do not
put certificate exports/passwords in this repository or build artifacts.

Dispatch with a successful **main** `windows-ci` run ID. The workflow authenticates
the repository, workflow path, event, head branch, success and main ancestry against
complete Git history before downloading the selected architecture's artifact. PR runs cannot reach the signer.
Trusted main scripts verify candidate/source checksums, expected immutable commit,
per-file inventory, executable identity and clean state before signing.

`Seal-Candidate.ps1` signs only Civic 89's executable and the generated installer/
uninstaller using SHA-256 and an HTTPS RFC3161 timestamp. Third-party DLL signatures
are retained. It rebuilds package hashes after signing, validates signer/thumbprint/
timestamp, and stages signed development candidates for review. The matching source
archive remains the build commit's archive. No release is automatically published.
See [SignTool](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool)
and [Inno signing](https://jrsoftware.org/ishelp/topic_setup_signtool.htm).

No trusted signing certificate/host is configured in the current workspace.
Positive production signing acceptance requires that provisioning. Invalid or
missing identities fail; an unsigned artifact is never represented as signed.

## Crash-safe portable update strategy

Release hosting will use this repository's GitHub Releases, with reviewed version
tags, matching source and checksums. Until stable published versioning and release
gates exist, there is **no automatic network update discovery/download/application**.
The implemented offline transaction accepts an explicitly selected ZIP and its
expected SHA-256 from the trusted delivery channel. A checksum proves integrity,
not publisher identity; authenticate the delivery/signature before using it.

The optional helpers require PowerShell 7; ordinary game launch does not. Run from
the extracted package or corresponding source tree:

```powershell
./tools/release/Update-Portable.ps1 -InstallRoot "$env:LOCALAPPDATA\Civic89Portable" `
  -Archive 'C:\Downloads\civic89-0.8.0-dev-windows-x64.zip' -ExpectedSha256 '<64 hex digits from trusted SHA256SUMS>'
./tools/release/Launch-Portable.ps1 -InstallRoot "$env:LOCALAPPDATA\Civic89Portable"
./tools/release/Update-Portable.ps1 -InstallRoot "$env:LOCALAPPDATA\Civic89Portable" -Rollback
```

Updates serialize through an exclusive lock, reject hostile ZIP paths/symlinks/
duplicates/oversized archives, verify every file/PE architecture/identity, launch
the new package in isolation and stage it in a fresh sibling version directory.
A flushed same-directory `current.json` replacement selects the complete release
atomically. The earlier directory and pointer backup remain. Interruption before
publication leaves the previous selection; after publication the new complete
directory is selected. Rollback switches to the preserved previous package.
Running old processes can finish using their old files; close/save before launching
the new version. No transaction changes city bytes or settings. Orphaned staging
and older directories are retained for inspection; no automatic cleanup deletes saves.
Rolling back to a version before 0.7.0 preserves `.c89` files but cannot open them;
use M7's explicit Classic export before playing the city in an older version.

Automated acceptance repeats a real delivery to exercise distinct generations,
rollback, a locked pointer during publication, wrong checksum, tampered content,
ZIP traversal/device/ADS/case-duplicate paths and preservation of Classic and Enhanced user cities.

## ARM64 and CI

On native ARM64 Windows with VS 2026 and ARM64 C++ tools:

```powershell
cmake --preset windows-arm64-release
cmake --build --preset windows-arm64-release -- /m
ctest --preset windows-arm64-release --output-on-failure
./tools/release/Package-Release.ps1 -BuildDirectory out/build/windows-arm64-release -Iscc '<verified ISCC.exe>'
```

The preset selects `ARM64` and `arm64-windows` target/host triplets. Cross-compiling
on an x64 workstation may override `-D VCPKG_HOST_TRIPLET=x64-windows`, but native
execution is required for test/packaging acceptance. x64 ASan presets remain x64.

`windows-ci` retains Debug, Release, headless ASan and application ASan and adds
native ARM64 Release on the [official VS 2026 ARM runner](https://github.com/actions/runner-images/blob/main/images/windows/Windows11-VS2026-Arm64-Readme.md).
Both Release jobs build, test, package, launch extracted delivery, exercise updates
and verify install/uninstall before uploading ZIP/installer/source/checksums.

## Public release checklist

Resolve the committed gates with evidence; finish physical clean-machine acceptance;
review the complete asset ledger; update version/CHANGELOG; merge only green checks;
build/sign the exact reviewed commit; inspect packages and matching source; obtain
release approval. Then create the reviewed tag and a draft GitHub Release containing
the signed architecture packages, source and SHA256SUMS. Review and publish the draft
as a separate authorised release action. No tag, release, purchase or signing identity
is created by the M6 or M7 engineering PRs.
