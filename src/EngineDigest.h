// Civic 89 state digests. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

class Budget;
class CityProperties;

// v1: stable little-endian integer fields, never pointers or wall-clock time.
std::uint64_t engineStateDigest(const CityProperties&, const Budget&);
