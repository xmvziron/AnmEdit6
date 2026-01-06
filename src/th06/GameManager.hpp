#pragma once

// #include <Windows.h>
// #include <d3d8.h>
// #include <d3dx8math.h>

#include "Chain.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"

namespace th06
{

#define PSCR_NUM_CHARS_SHOTTYPES 4
#define PSCR_NUM_STAGES 6
#define PSCR_NUM_DIFFICULTIES 4

#define CLRD_NUM_CHARACTERS 4

#define CATK_NUM_CAPTURES 64

#define GAME_REGION_TOP 16.0
#define GAME_REGION_LEFT 32.0

#define GAME_REGION_WIDTH 384.0
#define GAME_REGION_HEIGHT 448.0

#define GAME_REGION_RIGHT (GAME_REGION_LEFT + GAME_REGION_WIDTH)
#define GAME_REGION_BOTTOM (GAME_REGION_TOP + GAME_REGION_HEIGHT)

}; // namespace th06
