#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "MagmaTile.h"

MagmaTile::MagmaTile(int id) : Tile(id, Material::stone)
{	
	setLightEmission(0.2f);
}

void MagmaTile::tick(Level *level, int x, int y, int z, Random *random)
{
	int above = level->getTile(x, y + 1, z);
	if (above == Tile::water_Id || above == Tile::flowing_water_Id)
	{
		level->removeTile(x, y + 1, z);
		level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_FIZZ, 0.5f, 2.6f + (random->nextFloat() - random->nextFloat()) * 0.8f);

		for (int i = 0; i < 8; ++i)
		{
			float rx = x + 0.5f + (random->nextFloat() - random->nextFloat()) * 0.5f;
			float ry = y + 0.25f + random->nextFloat() * 0.25f;
			float rz = z + 0.5f + (random->nextFloat() - random->nextFloat()) * 0.5f;
			level->addParticle(eParticleType_largesmoke, rx, ry, rz, 0, 0, 0);
		}
	}
	level->addToTickNextTick(x, y, z, id, getTickDelay(level) + random->nextInt(10));
}

void MagmaTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	// rain -> smoke particles
	if (level->isRainingAt(x, y + 1, z))
	{
		for (int i = 0; i < 4; ++i)
		{
			float rx = x + random->nextFloat();
			float ry = y + 1.0f + random->nextFloat() * 0.5f;
			float rz = z + random->nextFloat();
			level->addParticle(eParticleType_largesmoke, rx, ry, rz, 0, 0, 0);
		}
	}
}

int MagmaTile::getTickDelay(Level *level)
{
	return 160;
}

int MagmaTile::getLightColor(LevelSource *level, int x, int y, int z, int tileId)
{
	return 0x0f000f0; 
}