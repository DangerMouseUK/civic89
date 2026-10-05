# Civic 89 - Project Status

**Current phase:** Local upstream baseline captured; GitHub origin and inherited-build verification pending  
**Status captured:** 5 October 2026  
**Current product name:** Civic 89  
**Repository root:** `C:\Dev\Projects\civic89`  
**External vcpkg root:** `C:\Dev\vcpkg` (required project convention; standalone checkout verification should be recorded separately if not already done)

## Authoritative local repository state

- Branch: `main`.
- Working tree: clean immediately after clone/remote rename.
- `.git`: confirmed at `C:\Dev\Projects\civic89\.git`.
- Baseline SHA: `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.
- Baseline tag: `upstream-sdlpp-baseline` (annotated, created locally).
- Preserved upstream remote: `upstream-sdlpp` -> `https://github.com/ldicker83/Micropolis-SDLPP.git`.
- New Civic 89 `origin`: **not yet configured** at this status capture.
- The actual cloned SHA exactly matches the SDLPP SHA used by the engineering audit.

## Bootstrap checklist

- [x] Install/select the Visual Studio Enterprise 2026 C++ development environment.
- [x] Confirm Git is installed.
- [x] Clone Micropolis-SDLPP directly into `C:\Dev\Projects\civic89` (no manual `git init`).
- [x] Confirm `.git` exists inside the project root.
- [x] Rename inherited `origin` to `upstream-sdlpp`.
- [x] Capture exact HEAD `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`.
- [x] Create annotated tag `upstream-sdlpp-baseline`.
- [ ] Create an empty private GitHub repository named `civic89` and add it as new `origin`.
- [ ] Push `main` and `upstream-sdlpp-baseline`.
- [ ] Verify the inherited `micropolis-cpp.sln` builds/runs before source changes.
- [ ] Add `AGENTS.md`, this status file and `project/` documentation; commit them on `main`.
- [ ] Create `feature/cmake-bootstrap`.
- [ ] Hand `C:\Dev\Projects\civic89` to the coding agent.

## First engineering task

**Baseline Bootstrap only:** introduce CMake/CMakePresets and reproducible vcpkg usage while preserving source layout, behaviour and the inherited Visual Studio build as a comparison path.

No UI redesign, no simulation changes, no `Eval()` removal and no major source moves in the first branch.
