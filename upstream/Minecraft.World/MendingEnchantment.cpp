#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "ItemInstance.h"
#include "MendingEnchantment.h"

MendingEnchantment::MendingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::all)
{
	setDescriptionId(IDS_ENCHANTMENT_MENDING);
}

int MendingEnchantment::getMinCost(int level)
{
	return level * 25;
}

int MendingEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 50;
}

int MendingEnchantment::getMaxLevel()
{
	return 1;
}

bool MendingEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	return item->isDamageableItem();
}