#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "Material.h"
#include "IconRegister.h"
#include "Facing.h"
#include "GrassPathTile.h"

GrassPathTile::GrassPathTile(int id) : Tile(id, Material::dirt), iconTop(nullptr), iconBottom(nullptr)
{
}

void GrassPathTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 15 / 16.0f, 1);
}

bool GrassPathTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool GrassPathTile::isCubeShaped()
{
	return false;
}

AABB *GrassPathTile::getAABB(Level *level, int x, int y, int z)
{
	return AABB::newTemp(x + 0, y + 0, z + 0, x + 1, y + 1, z + 1);
}

void GrassPathTile::registerIcons(IconRegister *iconRegister)
{
	iconTop    = iconRegister->registerIcon(L"grass_path_top");
	iconBottom = iconRegister->registerIcon(L"dirt");
	icon       = iconRegister->registerIcon(L"grass_path_side");
}

Icon *GrassPathTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return iconBottom;
	return icon;
}

void GrassPathTile::checkAndConvert(Level *level, int x, int y, int z)
{
	if (level->getMaterial(x, y + 1, z)->blocksMotion())
	{
		level->setTileAndUpdate(x, y, z, Tile::dirt_Id);
	}
}

void GrassPathTile::neighborChanged(Level *level, int x, int y, int z, int neighborId)
{
	checkAndConvert(level, x, y, z);
}

int GrassPathTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::dirt_Id;
}

int GrassPathTile::getResourceCount(Random *random)
{
	return 1;
}
