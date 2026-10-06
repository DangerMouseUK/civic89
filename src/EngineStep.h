// Civic 89 deterministic stepping. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class Budget;
class CityProperties;

// One inherited SimFrame phase, then date/message/evaluation update. Animation
// is separate: the GUI schedules it independently of simulation phases.
void stepEngine(CityProperties&, Budget&);
