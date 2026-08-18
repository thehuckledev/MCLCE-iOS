#pragma once

#include "Enchantment.h"

class Level;
class LivingEntity;
class BlockPos;

class FrostWalkerEnchantment : public Enchantment
{
public:
	FrostWalkerEnchantment(int id, int freq);

	virtual int getMinCost(int level) override;
	virtual int getMaxCost(int level) override;
	virtual int getMaxLevel() override;
	virtual bool isCompatibleWith(Enchantment *other) const override;
	virtual bool isTreasureOnly() override { return true; }

	static void freezeNearby(shared_ptr<LivingEntity> living, Level *level, int x, int y, int z, int enchLevel);
	static void onEntityMoved(shared_ptr<LivingEntity> living, Level *level, BlockPos pos, int enchLevel);
};
