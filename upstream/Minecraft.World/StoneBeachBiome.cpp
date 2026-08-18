#include "stdafx.h"
#include "StoneBeachBiome.h"
#include "net.minecraft.world.level.tile.h"
#include "BiomeDecorator.h"

StoneBeachBiome::StoneBeachBiome(int id) : Biome(id)
{
    this->topMaterial = static_cast<byte>(Tile::stone_Id);
    this->topMaterialData = 0;
    this->material = static_cast<byte>(Tile::stone_Id);
    this->materialData = 0;

    if (decorator != nullptr)
    {
        decorator->treeCount = 0;
        decorator->grassCount = 0;
        decorator->flowerCount = 0;
    }
}