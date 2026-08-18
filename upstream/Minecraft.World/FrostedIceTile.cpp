#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.stats.h"
#include "FrostedIceTile.h"
#include "Random.h"
#include "IconRegister.h"

const int FrostedIceTile::MAX_AGE;

FrostedIceTile::FrostedIceTile(int id) : IceTile(id)
{
	// Inherits ice friction/sound from IceTile. Scheduled ticks drive the melt.
	for (int i = 0; i < 4; i++) m_ageIcons[i] = nullptr;
}

void FrostedIceTile::registerIcons(IconRegister *iconRegister)
{
	m_ageIcons[0] = iconRegister->registerIcon(L"frosted_ice_0");
	m_ageIcons[1] = iconRegister->registerIcon(L"frosted_ice_1");
	m_ageIcons[2] = iconRegister->registerIcon(L"frosted_ice_2");
	m_ageIcons[3] = iconRegister->registerIcon(L"frosted_ice_3");
	icon = m_ageIcons[0];
}

Icon *FrostedIceTile::getTexture(int face, int data)
{
	int age = data;
	if (age < 0) age = 0;
	if (age > MAX_AGE) age = MAX_AGE;
	return m_ageIcons[age];
}

int FrostedIceTile::randomTickDelay(Random *random)
{
	return 20 + random->nextInt(21); // 20-40 ticks
}

void FrostedIceTile::onPlace(Level *level, int x, int y, int z)
{
	// Schedule the first melt check 60-120 ticks after forming
	level->addToTickNextTick(x, y, z, id, 60 + level->random->nextInt(61));
}

int FrostedIceTile::countIceNeighbours(Level *level, int x, int y, int z)
{
	static const int off[6][3] = {
		{ 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { -1, 0, 0 }, { 1, 0, 0 }
	};
	int count = 0;
	for (int i = 0; i < 6; i++)
	{
		if (level->getTile(x + off[i][0], y + off[i][1], z + off[i][2]) == id)
		{
			if (++count >= 4) return count;
		}
	}
	return count;
}

void FrostedIceTile::meltToWater(Level *level, int x, int y, int z)
{
	if (level->dimension->ultraWarm)
	{
		level->removeTile(x, y, z);
		return;
	}
	level->setTileAndUpdate(x, y, z, Tile::water_Id);
	level->updateNeighborsAt(x, y, z, Tile::water_Id);
}

bool FrostedIceTile::slightlyMelt(Level *level, int x, int y, int z, Random *random, bool spreadToNeighbours)
{
	int age = level->getData(x, y, z);
	if (age < MAX_AGE)
	{
		level->setData(x, y, z, age + 1, Tile::UPDATE_CLIENTS);
		level->addToTickNextTick(x, y, z, id, randomTickDelay(random));
		return false;
	}
	meltToWater(level, x, y, z);

	if (spreadToNeighbours)
	{
		static const int off[6][3] = {
			{ 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { -1, 0, 0 }, { 1, 0, 0 }
		};
		for (int i = 0; i < 6; i++)
		{
			int nx = x + off[i][0], ny = y + off[i][1], nz = z + off[i][2];
			if (level->getTile(nx, ny, nz) == id)
				slightlyMelt(level, nx, ny, nz, random, false);
		}
	}
	return true;
}

void FrostedIceTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	int age = level->getData(x, y, z);

	int light = level->getRawBrightness(x, y, z);


	if ((random->nextInt(3) == 0 || countIceNeighbours(level, x, y, z) < 4)
		&& light > 11 - age - Tile::lightBlock[id])
	{
		slightlyMelt(level, x, y, z, random, true);
	}
	else
	{
		level->addToTickNextTick(x, y, z, id, randomTickDelay(random));
	}
}

void FrostedIceTile::neighborChanged(Level *level, int x, int y, int z, int neighborId)
{
	// If a neighbouring frosted ice melts away leaving us poorly supported, melt too
	if (neighborId == id && countIceNeighbours(level, x, y, z) < 2)
	{
		meltToWater(level, x, y, z);
	}
}

int FrostedIceTile::getResourceCount(Random *random)
{
	return 0; // never drops anything, even with silk touch
}

void FrostedIceTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	// Frosted ice yields nothing and turns to water like normal ice
	if (level->dimension->ultraWarm)
	{
		level->removeTile(x, y, z);
		return;
	}
	Material *below = level->getMaterial(x, y - 1, z);
	if (below->blocksMotion() || below->isLiquid())
	{
		level->setTileAndUpdate(x, y, z, Tile::water_Id);
	}
}
