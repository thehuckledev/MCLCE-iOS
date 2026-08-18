#pragma once
#include "Biome.h"

class StoneBeachBiome : public Biome
{
public:
    StoneBeachBiome(int id);
    virtual bool isFoggy() const { return false; }
    virtual bool isNatural() const { return true; }
};