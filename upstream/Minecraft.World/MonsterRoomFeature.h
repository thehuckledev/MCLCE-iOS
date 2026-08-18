#pragma once
#include "Feature.h"
#include "Material.h"

class WeighedTreasure;

class MonsterRoomFeature : public Feature
{
public:
	virtual bool place(Level *level, Random *random, int x, int y, int z);

private:
	wstring randomEntityId(Random *random);
};
