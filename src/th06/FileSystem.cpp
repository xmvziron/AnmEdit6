#include <stdio.h>

/* stub FileSystem.cpp, just enough to get it compiling */

namespace th06
{

u32 g_LastFileSize;

u8 *FileSystem::OpenPath(const char *filepath, int isExternalResource)
{
    u8 *result = (u8 *) SDL_LoadFile(filepath, (size_t *) &g_LastFileSize);
    
    return result;
}

FILE *FileSystem::FopenUTF8(const char *filepath, const char *mode)
{
    return fopen(filepath, mode);
}

} /* namespace th06 */