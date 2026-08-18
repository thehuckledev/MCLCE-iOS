#pragma once

#include "Enchantment.h"

class MendingEnchantment : public Enchantment
{
public:
	MendingEnchantment(int id, int freq);

	virtual int getMinCost(int level) override;
	virtual int getMaxCost(int level) override;
	virtual int getMaxLevel() override;
	virtual bool isTreasureOnly() override { return true; };
	virtual bool canEnchant(shared_ptr<ItemInstance> item) override;
};
