#include "../Minecraft.World/LootTableManager.h"
#include "../Minecraft.World/Biome.h"
#include "../Minecraft.World/FishingHelper.h"
#include "../Minecraft.World/ItemInstance.h"
#include "../Minecraft.World/EnchantmentHelper.h"
#include "net.minecraft.world.item.h"
#include <memory>

FishingHelper* FishingHelper::getInstance()
{
	static FishingHelper instance;
	return &instance;
}

FishingHelper::FishingHelper()
{
}

CatchType FishingHelper::getRandCatchType(int luckLevel, int lureLevel, Random* random)
{
	float randFloat = random->nextFloat();
	float junkChance = 0.1F - (float)luckLevel * 0.025F - (float)lureLevel * 0.01F; // default 10% chance, affected by lure and luck of the sea
	float treasureChance = 0.05F + (float)luckLevel * 0.01F - (float)lureLevel * 0.01F; // default 5% chance, affected by lure and luck of the sea
	junkChance = clamp(junkChance, 0.0F, 1.0F);
	treasureChance = clamp(treasureChance, 0.0F, 1.0F);

	if (randFloat < junkChance)
	{
		return CatchType::JUNK;
	}
	else
	{
		randFloat -= junkChance;
		if (randFloat < treasureChance)
		{
			return CatchType::TREASURE;
		}
		return CatchType::FISH;
	}
}

std::shared_ptr<ItemInstance> FishingHelper::handleCatch(const LootTableDropResult &drop, Random* random)
{
	std::shared_ptr<ItemInstance> itemInstance = std::make_shared<ItemInstance>(
		drop.itemId, drop.count, drop.data
	);

	if (drop.damageFraction > 0.0)
	{
		itemInstance->setAuxValue((int)((double)itemInstance->getMaxDamage() * drop.damageFraction));
	}

	if (drop.enchantLevels > 0)
	{
		EnchantmentHelper::enchantItem(random, itemInstance, drop.enchantLevels);
	}

	return itemInstance;
}

std::shared_ptr<ItemInstance> FishingHelper::getCatch(int luckLevel, int lureLevel, Random* random)
{
	CatchType catchType = getRandCatchType(luckLevel, lureLevel, random);

	std::string tableName;
	switch (catchType)
	{
	case CatchType::JUNK:      tableName = "gameplay/fishing/junk"; break;
	case CatchType::TREASURE:  tableName = "gameplay/fishing/treasure"; break;
	case CatchType::FISH:
	default:                   tableName = "gameplay/fishing/fish"; break;
	}

	std::vector<LootTableDropResult> drops = LootTableManager::Get().ResolveDrops(
		tableName, false, 0,
		[random](int bound) { return random->nextInt(bound); });

	if (drops.empty())
	{
		// just return a fish as this should NOT happen
		return std::make_shared<ItemInstance>(Item::raw_fish);
	}

	return handleCatch(drops.front(), random);
}