#pragma once

class Level;
class Dimension;

inline bool isReasonablePointer(const void *ptr)
{
    if (ptr == nullptr)
        return false;

    unsigned long long addr = reinterpret_cast<unsigned long long>(ptr);
    if (addr <= 0x10000ULL || addr > 0x00007FFFFFFFFFFFULL)
        return false;
    if ((addr & 0x7ULL) != 0ULL)
        return false;
    if ((addr & 0xFFFFFFFFULL) == 0ULL)
        return false;
    return true;
}

inline bool isReasonableLevelPointer(Level *lev)
{
    return isReasonablePointer(lev);
}

inline bool isReasonableDimensionPointer(Dimension *dim)
{
    return isReasonablePointer(dim);
}
