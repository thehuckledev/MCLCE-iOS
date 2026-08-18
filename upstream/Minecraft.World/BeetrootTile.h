#pragma once

#include "CropTile.h"

class BeetrootTile : public CropTile
{
	friend class ChunkRebuildData;
private:
	Icon *icons[4];

public:
	BeetrootTile(int id);

	Icon *getTexture(int face, int data);

protected:
	int getBaseSeedId();
	int getBasePlantId();

public:
	void tick(Level *level, int x, int y, int z, Random *random);
	void growCrops(Level *level, int x, int y, int z);
	int getResource(int data, Random *random, int playerBonusLevel);
	void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);
	void registerIcons(IconRegister *iconRegister);
};
