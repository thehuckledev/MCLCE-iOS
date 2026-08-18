#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.biome.h"

ShoreLayer::ShoreLayer(int64_t seed, shared_ptr<Layer> parent, int64_t seedMixup) : Layer(seedMixup)
{
    this->parent = parent;
}

bool ShoreLayer::isJungleCompatible(int id)
{
    if (id == Biome::jungle->id || id == Biome::jungleHills->id || id == Biome::jungleEdge->id ||
        id == Biome::jungleM->id || id == Biome::jungleEdgeM->id)
        return true;
    if (id == Biome::forest->id || id == Biome::taiga->id)
        return true;
    if (isOcean(id))
        return true;
    return false;
}

bool ShoreLayer::isMesaBiome(int id)
{
    return id == Biome::mesa->id || id == Biome::mesaPlateauF->id || id == Biome::mesaPlateau->id ||
           id == Biome::mesaBryce->id || id == Biome::mesaPlateauFM->id || id == Biome::mesaPlateauM->id;
}

void ShoreLayer::replaceIfNeighborOcean(intArray& b, intArray& result, int x, int y, int w, int stride, int center, int target)
{
    if (!isOcean(center))
    {
        int n  = b[(x + 1) + (y + 0) * stride];
        int e  = b[(x + 2) + (y + 1) * stride];
        int w1 = b[(x + 0) + (y + 1) * stride];
        int s  = b[(x + 1) + (y + 2) * stride];
        if (isOcean(n) || isOcean(e) || isOcean(w1) || isOcean(s))
        {
            result[x + y * w] = target;
            return;
        }
    }
    result[x + y * w] = center;
}

intArray ShoreLayer::getArea(int xo, int yo, int w, int h)
{
    intArray b = parent->getArea(xo - 1, yo - 1, w + 2, h + 2);
    intArray result = IntCache::allocate(w * h);
    int stride = w + 2;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            initRandom(x + xo, y + yo);
            int old = b[(x + 1) + (y + 1) * stride];

            int n  = b[(x + 1) + (y + 0) * stride];
            int e  = b[(x + 2) + (y + 1) * stride];
            int w1 = b[(x + 0) + (y + 1) * stride];
            int s  = b[(x + 1) + (y + 2) * stride];

            if (old == Biome::mushroomIsland->id)
            {
                // decompile checks plain ocean only, NOT deepOcean, for this branch
                bool nearOcean = (n == Biome::ocean->id || e == Biome::ocean->id ||
                                   w1 == Biome::ocean->id || s == Biome::ocean->id);
                result[x + y * w] = nearOcean ? Biome::mushroomIslandShore->id : old;
                continue;
            }

            if (old == Biome::jungle->id || old == Biome::jungleHills->id ||
                old == Biome::jungleM->id)
            {
                bool allCompatible = isJungleCompatible(n) && isJungleCompatible(e) &&
                                      isJungleCompatible(w1) && isJungleCompatible(s);
                if (!allCompatible)
                {
                    result[x + y * w] = Biome::jungleEdge->id;
                }
                else
                {
                    bool nearOcean = (isOcean(n) || isOcean(e) || isOcean(w1) || isOcean(s));
                    result[x + y * w] = nearOcean ? Biome::beaches->id : old;
                }
                continue;
            }

            if (old == Biome::extremeHills->id || old == Biome::extremeHills_plus->id ||
                old == Biome::smallerExtremeHills->id)
            {
                replaceIfNeighborOcean(b, result, x, y, w, stride, old, Biome::stoneBeach->id);
                continue;
            }

            Biome* biome = Biome::getBiome(old);
            if (biome != nullptr && biome->hasSnow())
            {
                replaceIfNeighborOcean(b, result, x, y, w, stride, old, Biome::coldBeach->id);
                continue;
            }

            if (old == Biome::mesa->id || old == Biome::mesaPlateauF->id)
            {
                bool nearOcean = (isOcean(n) || isOcean(e) || isOcean(w1) || isOcean(s));
                if (nearOcean)
                {
                    result[x + y * w] = Biome::beaches->id;
                }
                else
                {
                    bool allMesa = isMesaBiome(n) && isMesaBiome(e) && isMesaBiome(w1) && isMesaBiome(s);
                    result[x + y * w] = allMesa ? old : Biome::desert->id;
                }
                continue;
            }

            if (old != Biome::ocean->id && old != Biome::deepOcean->id &&
                old != Biome::river->id && old != Biome::swampland->id)
            {
                bool nearOcean = (isOcean(n) || isOcean(e) || isOcean(w1) || isOcean(s));
                result[x + y * w] = nearOcean ? Biome::beaches->id : old;
                continue;
            }

            result[x + y * w] = old;
        }
    }
    return result;
}
