#include "Supervisor.hpp"

/* yanked from th06/src/Supervisor.cpp */

namespace th06
{

SDL_Surface *g_TextBufferSurface;

void Supervisor::TickTimer(i32 *frames, f32 *subframes)
{
    if (this->framerateMultiplier <= 0.99f)
    {
        *subframes = *subframes + this->effectiveFramerateMultiplier;
        if (*subframes >= 1.0f)
        {
            *frames = *frames + 1;
            *subframes = *subframes - 1.0f;
        }
    }
    else
    {
        *frames = *frames + 1;
    }
}

} /* namespaace th06 */