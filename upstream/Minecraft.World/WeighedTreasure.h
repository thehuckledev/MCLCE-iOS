#pragma once

#include <vector>
#include "WeighedRandom.h"
#include "LootTableManager.h"

class WeighedTreasure : public WeighedRandomItem
{
private:
	shared_ptr<ItemInstance> item;
	int minCount;
	int maxCount;

public:
	WeighedTreasure(int itemId, int auxValue, int minCount, int maxCount, int weight);
	WeighedTreasure(shared_ptr<ItemInstance> item, int minCount, int maxCount, int weight);

	static void addChestItems(Random *random, const std::vector<LootTableDropResult> &drops, shared_ptr<Container> dest);
	static void addDispenserItems(Random *random, const std::vector<LootTableDropResult> &drops, shared_ptr<DispenserTileEntity> dest);
	static WeighedTreasureArray addToTreasure(WeighedTreasureArray items, WeighedTreasure *extra);
};