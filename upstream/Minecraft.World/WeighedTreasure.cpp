#include "stdafx.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "LootTableManager.h"
#include "WeighedRandom.h"
#include "WeighedTreasure.h"
#include "EnchantmentHelper.h"

#include <vector>

namespace
{
    template <typename ContainerT>
    void FillContainerFromDrops(Random *random, const std::vector<LootTableDropResult> &drops, shared_ptr<ContainerT> dest)
    {
        for (const LootTableDropResult &drop : drops)
        {
            if (drop.itemId == 0 || drop.count <= 0)
            {
                continue;
            }

            shared_ptr<ItemInstance> item = std::make_shared<ItemInstance>(drop.itemId, 1, drop.data);

            if (drop.damageFraction > 0.0)
            {
                item->setAuxValue((int)((double)item->getMaxDamage() * drop.damageFraction));
            }

            if (drop.enchantLevels > 0)
            {
                EnchantmentHelper::enchantItem(random, item, drop.enchantLevels);
            }

            if (item->getMaxStackSize() >= drop.count)
            {
                shared_ptr<ItemInstance> copy = item->copy();
                copy->count = drop.count;
                dest->setItem(random->nextInt(dest->getContainerSize()), copy);
            }
            else
            {
                // use multiple slots if possible
                for (int c = 0; c < drop.count; c++)
                {
                    shared_ptr<ItemInstance> copy = item->copy();
                    copy->count = 1;
                    dest->setItem(random->nextInt(dest->getContainerSize()), copy);
                }
            }
        }
    }
}

WeighedTreasure::WeighedTreasure(int itemId, int auxValue, int minCount, int maxCount, int weight) : WeighedRandomItem(weight)
{
    this->item = std::make_shared<ItemInstance>(itemId, 1, auxValue);
    this->minCount = minCount;
    this->maxCount = maxCount;
}

WeighedTreasure::WeighedTreasure(shared_ptr<ItemInstance> item, int minCount, int maxCount, int weight) : WeighedRandomItem(weight)
{
    this->item = item;
    this->minCount = minCount;
    this->maxCount = maxCount;
}

void WeighedTreasure::addChestItems(Random *random, const std::vector<LootTableDropResult> &drops, shared_ptr<Container> dest)
{
    FillContainerFromDrops(random, drops, dest);
}

void WeighedTreasure::addDispenserItems(Random *random, const std::vector<LootTableDropResult> &drops, shared_ptr<DispenserTileEntity> dest)
{
    FillContainerFromDrops(random, drops, dest);
}

WeighedTreasureArray WeighedTreasure::addToTreasure(WeighedTreasureArray items, WeighedTreasure *extra)
{
    WeighedTreasureArray result(items.length + 1);
    int i = 0;

    for (int j = 0; j < items.length; j++)
    {
        result[i++] = items[j];
    }

    result[i++] = extra;

    return result;
}