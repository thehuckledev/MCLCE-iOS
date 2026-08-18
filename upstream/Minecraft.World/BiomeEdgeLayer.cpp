// TODO: fix cold-swampland edge-case scenario
#include "stdafx.h"
#include "BiomeEdgeLayer.h"
#include "net.minecraft.world.level.biome.h"
#include "IntCache.h"

BiomeEdgeLayer::BiomeEdgeLayer(int64_t seed, shared_ptr<Layer> parent, int64_t seedMixup) : Layer(seedMixup)
{
    this->parent = parent;
}

intArray BiomeEdgeLayer::getArea(int xo, int yo, int w, int h)
{
    intArray b = parent->getArea(xo - 1, yo - 1, w + 2, h + 2);
    intArray result = IntCache::allocate(w * h);
    int stride = w + 2;

    for (int iy = 0; iy < h; ++iy)
    {
        for (int ix = 0; ix < w; ++ix)
        {
            initRandom(ix + xo, iy + yo);
            int center = b[(ix + 1) + (iy + 1) * stride];

            if (checkEdge(b, result, ix, iy, stride, Biome::extremeHills->id, Biome::smallerExtremeHills->id, center))
                continue;
            if (checkEdgeStrict(b, result, ix, iy, stride, Biome::mesaPlateauF->id, Biome::mesa->id, center))
                continue;
            if (checkEdgeStrict(b, result, ix, iy, stride, Biome::mesaPlateau->id, Biome::mesa->id, center))
                continue;
            if (checkEdgeStrict(b, result, ix, iy, stride, Biome::megaTaiga->id, Biome::taiga->id, center))
                continue;

            int n  = b[(ix + 1) + (iy + 0) * stride];
            int e  = b[(ix + 2) + (iy + 1) * stride];
            int w1 = b[(ix + 0) + (iy + 1) * stride];
            int s  = b[(ix + 1) + (iy + 2) * stride];

            if (center == Biome::desert->id)
            {
                bool nearIce = (n == Biome::iceFlats->id || e == Biome::iceFlats->id ||
                                w1 == Biome::iceFlats->id || s == Biome::iceFlats->id);
                result[ix + iy * w] = nearIce ? Biome::extremeHills_plus->id : center;
            }
            else if (center == Biome::swampland->id)
            {
                bool nearDesert    = (n == Biome::desert->id || e == Biome::desert->id || w1 == Biome::desert->id || s == Biome::desert->id);
                bool nearColdTaiga = (n == Biome::coldTaiga->id || e == Biome::coldTaiga->id || w1 == Biome::coldTaiga->id || s == Biome::coldTaiga->id);
                bool nearIceFlats  = (n == Biome::iceFlats->id || e == Biome::iceFlats->id || w1 == Biome::iceFlats->id || s == Biome::iceFlats->id);

                if (nearDesert || nearColdTaiga || nearIceFlats)
                {
                    result[ix + iy * w] = Biome::plains->id;
                }
                else
                {
                    bool nearJungle = (n == Biome::jungle->id || e == Biome::jungle->id || w1 == Biome::jungle->id || s == Biome::jungle->id);
                    result[ix + iy * w] = nearJungle ? Biome::jungleEdge->id : center;
                }
            }
            else
            {
                result[ix + iy * w] = center;
            }
        }
    }
    return result;
}

bool BiomeEdgeLayer::checkEdge(intArray& b, intArray& result, int x, int y, int stride, int biome, int target, int center)
{
    if (!isSame(center, biome)) return false;

    int n  = b[(x + 1) + (y + 0) * stride];
    int e  = b[(x + 2) + (y + 1) * stride];
    int w1 = b[(x + 0) + (y + 1) * stride];
    int s  = b[(x + 1) + (y + 2) * stride];

    bool interior = isValidTemperatureEdge(n, biome) && isValidTemperatureEdge(e, biome) &&
                     isValidTemperatureEdge(w1, biome) && isValidTemperatureEdge(s, biome);

    result[x + y * (stride - 2)] = interior ? center : target;
    return true;
}

bool BiomeEdgeLayer::checkEdgeStrict(intArray& b, intArray& result, int x, int y, int stride, int biome, int target, int center)
{
    if (center != biome) return false;

    int n  = b[(x + 1) + (y + 0) * stride];
    int e  = b[(x + 2) + (y + 1) * stride];
    int w1 = b[(x + 0) + (y + 1) * stride];
    int s  = b[(x + 1) + (y + 2) * stride];

    bool interior = isSame(n, biome) && isSame(e, biome) && isSame(w1, biome) && isSame(s, biome);

    result[x + y * (stride - 2)] = interior ? center : target;
    return true;
}

bool BiomeEdgeLayer::isValidTemperatureEdge(int neighbor, int target)
{
    if (isSame(neighbor, target)) return true;
    Biome* a = Biome::getBiome(neighbor);
    Biome* b = Biome::getBiome(target);
    if (a != nullptr && b != nullptr)
    {
        int catA = a->getTemperatureCategory();
        int catB = b->getTemperatureCategory();
        return catA == catB || catA == 2 || catB == 2;
    }
    return false;
}