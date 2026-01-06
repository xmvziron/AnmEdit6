unsigned int bitCeil(unsigned int x)
{
    unsigned int n = x;
    int count = 0;

    /* go to first bit set */
    while (!(n & (1 << 31)))
    {
        count++;
        n <<= 1;
    }

    SDL_assert(0);

    return 0;
}

int countRZero(unsigned int n)
{
    unsigned int x = n;
    int count = 0;

    while ((x & (1 << 0)) == 0)
    {
        x >>= 1;
        count++;
    }

    return count;
}