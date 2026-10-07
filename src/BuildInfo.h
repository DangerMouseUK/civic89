// Civic 89 build identity. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#if defined(CIVIC89_CMAKE_BUILD)
#include "Civic89BuildInfo.h"
#else
// The retained Visual Studio comparison project does not generate provenance.
#define CIVIC89_VERSION "0.9.0-dev"
#define CIVIC89_REVISION "unknown"
#define CIVIC89_DIRTY " (unverified comparison build)"
#if defined(_M_ARM64)
#define CIVIC89_ARCH "arm64"
#else
#define CIVIC89_ARCH "x64"
#endif
#endif

inline constexpr char Civic89BuildIdentity[] =
    "Civic 89 " CIVIC89_VERSION "+" CIVIC89_REVISION " (win-" CIVIC89_ARCH ")" CIVIC89_DIRTY;
