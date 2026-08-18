#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "Material.h"
#include "Mth.h"
#include "BlockPos.h"
#include "FrostWalkerEnchantment.h"
#include "EnchantmentHelper.h"

FrostWalkerEnchantment::FrostWalkerEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
	setDescriptionId(IDS_ENCHANTMENT_FROST_WALKER);
}

int FrostWalkerEnchantment::getMinCost(int level)
{
	return level * 10;
}

int FrostWalkerEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 15;
}

int FrostWalkerEnchantment::getMaxLevel()
{
	return 2;
}

bool FrostWalkerEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (!Enchantment::isCompatibleWith(other)) return false;
	return other != Enchantment::waterWalker;
}

bool EnchantmentHelper::hasFrostWalker(shared_ptr<LivingEntity> living)
{
	return getFrostWalker(living) != 0;
}

void FrostWalkerEnchantment::onEntityMoved(shared_ptr<LivingEntity> living, Level *level, BlockPos pos, int enchLevel)
{
	if (!living->onGround) return;

	Player *player = dynamic_cast<Player *>(living.get());
	if (player != nullptr && player->abilities.flying) return;

	int radius = enchLevel + 2;
	if (radius > 16) radius = 16;
	float f = static_cast<float>(radius);

	BlockPos minPos = pos.offset(-radius, -1, -radius);
	BlockPos maxPos = pos.offset(radius, -1, radius);

	for (int x = minPos.getX(); x <= maxPos.getX(); ++x)
	{
		for (int y = minPos.getY(); y <= maxPos.getY(); ++y)
		{
			for (int z = minPos.getZ(); z <= maxPos.getZ(); ++z)
			{
				double dx = (x + 0.5) - living->x;
				double dy = (y + 0.5) - living->y;
				double dz = (z + 0.5) - living->z;
				if (dx * dx + dy * dy + dz * dz > static_cast<double>(f * f)) continue;

				if (level->getTile(x, y + 1, z) != 0) continue;

				Material *ground = level->getMaterial(x, y, z);
				if (ground != Material::water) continue;
				if (level->getData(x, y, z) != 0) continue;

				bool canPlace = level->mayPlace(Tile::frosted_ice_Id, x, y, z, false, 0, nullptr, nullptr);
				if (!canPlace) continue;

				level->setTileAndData(x, y, z, Tile::frosted_ice_Id, 0, Tile::UPDATE_ALL);

				int ticks = Mth::nextInt(living->getRandom(), 60, 120);
				level->addToTickNextTick(x, y, z, Tile::frosted_ice_Id, ticks);
			}
		}
	}
}