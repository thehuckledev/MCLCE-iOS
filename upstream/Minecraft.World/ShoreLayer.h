#pragma once
#include "Layer.h"

class ShoreLayer : public Layer
{
private:
    static bool isJungleCompatible(int id);
    static bool isMesaBiome(int id);
    void replaceIfNeighborOcean(intArray& b, intArray& result, int x, int y, int w, int stride, int center, int target);
public:
    ShoreLayer(int64_t seed, shared_ptr<Layer> parent, int64_t seedMixup);
    virtual intArray getArea(int xo, int yo, int w, int h);
};