# Civic 89 Upstreams

## Primary inherited upstream

**Micropolis-SDLPP**  
Repository: https://github.com/ldicker83/Micropolis-SDLPP  
Local remote name: `upstream-sdlpp`  
Role: preserved Git history and native Windows/SDL3 baseline.

**Engineering audit SHA:** `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`  
**Actual Civic 89 cloned baseline SHA:** `9c4e85a0decd57ba6f76d9e1ec82461940ecc3ad`  
**Baseline tag:** `upstream-sdlpp-baseline`  
**Captured:** 5 October 2026

The actual clone and the earlier engineering audit resolve to the same commit. The baseline tag was created locally after the inherited `origin` was renamed to `upstream-sdlpp`. The working tree was clean at capture. The tag is a fixed provenance marker and must not be moved when upstream advances.

At this capture point, a new Civic 89 `origin` had not yet been configured.

**Subsequent verified state, 5 October 2026:** `origin` is `https://github.com/DangerMouseUK/civic89.git`; its `main`, `feature/cmake-bootstrap` and fixed baseline tag are present. The upstream fetch remote is retained and its push URL is `DISABLED`. Current build and font baseline evidence are in `PROJECT_STATUS.md` and `RUNTIME_ASSETS.md`.

## Reference upstream

**MicropolisCore**  
Repository: https://github.com/SimHacker/MicropolisCore  
Audited reference SHA: `f9ae6a57bbe5f5ff94c149bccb3015757f18241d`  
Role: reference source for simulation fixes, `.cty` handling, tests and historical/architectural comparison.

Do not merge MicropolisCore repository history wholesale. Port selected fixes deliberately and cite the originating issue/commit in the Civic 89 commit/PR.

## Original lineage

**Micropolis / open-source original SimCity lineage**  
Repository: https://github.com/SimHacker/micropolis

Retain inherited GPL/additional-term notices and copyright provenance.
