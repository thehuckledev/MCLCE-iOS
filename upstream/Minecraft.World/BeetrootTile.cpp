#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "BeetrootTile.h"

BeetrootTile::BeetrootTile(int id) : CropTile(id)
{
}

Icon *BeetrootTile::getTexture(int face, int data)
{
	if (data < 0 || data > 3) data = 3;
	return icons[data];
}

int BeetrootTile::getBaseSeedId()
{
	return Item::beetroot_seeds_Id;
}

int BeetrootTile::getBasePlantId()
{
	return Item::beetroot_Id;
}

void BeetrootTile::tick(Level *level, int x, int y, int z, Random *random)
{
	Bush::tick(level, x, y, z, random);
	if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6)
	{
		int age = level->getData(x, y, z);
		if (age < 3)
		{
			if (random->nextInt(3) != 0)
			{
				float growthSpeed = 1.0f;
				if (random->nextInt(static_cast<int>(25 / growthSpeed) + 1) == 0)
				{
					age++;
					level->setData(x, y, z, age, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}
}

void BeetrootTile::growCrops(Level *level, int x, int y, int z)
{
	int stage = level->getData(x, y, z) + 1;
	if (stage > 3) stage = 3;
	level->setData(x, y, z, stage, Tile::UPDATE_CLIENTS);
}

int BeetrootTile::getResource(int data, Random *random, int playerBonusLevel)
{
	if (data == 3)
	{
		return getBasePlantId();
	}
	return getBaseSeedId();
}

void BeetrootTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	Bush::spawnResources(level, x, y, z, data, odds, 0);

	if (level->isClientSide)
	{
		return;
	}
	if (data >= 3)
	{
		int seedCount = 1 + level->random->nextInt(4);
		for (int i = 0; i < seedCount; i++)
		{
			popResource(level, x, y, z, std::make_shared<ItemInstance>(getBaseSeedId(), 1, 0));
		}
	}
}

void BeetrootTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < 4; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + std::to_wstring(i));
	}
}
