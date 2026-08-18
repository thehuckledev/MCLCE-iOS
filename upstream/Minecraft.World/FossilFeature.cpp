#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "RotatedPillarTile.h"
#include "FossilFeature.h"

struct FossilBlock
{
	signed char x, y, z;
	signed char data;
};

struct FossilStructure
{
	int width, height, length;
	const FossilBlock *blocks;
	int blockCount;
};

static const int AXIS_Y = RotatedPillarTile::FACING_Y;
static const int AXIS_X = RotatedPillarTile::FACING_X;
static const int AXIS_Z = RotatedPillarTile::FACING_Z;
static const int COAL   = -1;

static const FossilBlock skull01[] = {
	{1,1,0,AXIS_Y},{4,1,0,AXIS_Y},{1,2,0,AXIS_Y},{2,2,0,AXIS_Y},{3,2,0,AXIS_Y},{4,2,0,AXIS_Y},
	{2,3,0,AXIS_X},{3,3,0,AXIS_X},
	{1,0,1,AXIS_X},{2,0,1,AXIS_X},{3,0,1,AXIS_X},{4,0,1,AXIS_X},
	{0,1,1,AXIS_Y},{5,1,1,AXIS_Y},{0,2,1,AXIS_Y},{5,2,1,AXIS_Y},{0,3,1,AXIS_Y},{5,3,1,AXIS_Y},
	{1,4,1,AXIS_X},{2,4,1,AXIS_X},{3,4,1,AXIS_X},{4,4,1,AXIS_X},
	{1,0,2,AXIS_X},{2,0,2,AXIS_X},{3,0,2,AXIS_X},{4,0,2,AXIS_X},
	{0,1,2,AXIS_Z},{5,1,2,AXIS_Z},{0,2,2,AXIS_Z},{5,2,2,AXIS_Z},{0,3,2,AXIS_Z},{5,3,2,AXIS_Z},
	{1,4,2,AXIS_X},{2,4,2,AXIS_X},{3,4,2,AXIS_X},{4,4,2,AXIS_X},
	{1,0,3,AXIS_X},{2,0,3,AXIS_X},{3,0,3,AXIS_X},{4,0,3,AXIS_X},
	{0,1,3,AXIS_Z},{5,1,3,AXIS_Z},{0,2,3,AXIS_Z},{5,2,3,AXIS_Z},{0,3,3,AXIS_Z},{5,3,3,AXIS_Z},
	{1,4,3,AXIS_X},{2,4,3,AXIS_X},{3,4,3,AXIS_X},{4,4,3,AXIS_X},
	{1,0,4,AXIS_X},{4,0,4,AXIS_X},
	{0,1,4,AXIS_Z},{5,1,4,AXIS_Z},{0,2,4,AXIS_Z},{5,2,4,AXIS_Z},{0,3,4,AXIS_Z},{5,3,4,AXIS_Z},
	{1,4,4,AXIS_X},{2,4,4,AXIS_X},{3,4,4,AXIS_X},{4,4,4,AXIS_X},
	{1,0,5,AXIS_X},{4,0,5,AXIS_X},
	{0,1,5,AXIS_Y},{5,1,5,AXIS_Y},{0,2,5,AXIS_Y},{5,2,5,AXIS_Y},{0,3,5,AXIS_Y},{5,3,5,AXIS_Y},
	{1,4,5,AXIS_X},{2,4,5,AXIS_X},{3,4,5,AXIS_X},{4,4,5,AXIS_X},
	{1,1,6,AXIS_Y},{2,1,6,AXIS_X},{3,1,6,AXIS_X},{4,1,6,AXIS_Y},
	{1,2,6,AXIS_Y},{2,2,6,AXIS_X},{3,2,6,AXIS_X},{4,2,6,AXIS_Y},
	{1,3,6,AXIS_Y},{2,3,6,AXIS_X},{3,3,6,AXIS_X},{4,3,6,AXIS_Y},
};

static const FossilBlock skull02[] = {
	{1,1,0,AXIS_Y},{5,1,0,AXIS_Y},{1,2,0,AXIS_Y},{2,2,0,AXIS_X},{3,2,0,AXIS_X},{4,2,0,AXIS_X},{5,2,0,AXIS_Y},
	{1,3,0,AXIS_Y},{3,3,0,AXIS_Y},{5,3,0,AXIS_Y},{2,4,0,AXIS_X},{4,4,0,AXIS_X},
	{1,0,1,AXIS_X},{2,0,1,AXIS_X},{3,0,1,AXIS_X},{4,0,1,AXIS_X},{5,0,1,AXIS_X},
	{0,1,1,AXIS_Y},{6,1,1,AXIS_Y},{0,2,1,AXIS_Y},{6,2,1,AXIS_Y},{0,3,1,AXIS_Y},{6,3,1,AXIS_Y},
	{1,4,1,AXIS_X},{2,4,1,AXIS_X},{3,4,1,AXIS_X},{4,4,1,AXIS_X},{5,4,1,AXIS_X},
	{1,0,2,AXIS_X},{2,0,2,AXIS_X},{3,0,2,AXIS_X},{4,0,2,AXIS_X},{5,0,2,AXIS_X},
	{0,1,2,AXIS_Z},{6,1,2,AXIS_Z},{0,2,2,AXIS_Z},{6,2,2,AXIS_Z},{0,3,2,AXIS_Z},{6,3,2,AXIS_Z},
	{1,4,2,AXIS_X},{2,4,2,AXIS_X},{3,4,2,AXIS_X},{4,4,2,AXIS_X},{5,4,2,AXIS_X},
	{1,0,3,AXIS_X},{2,0,3,AXIS_X},{3,0,3,AXIS_X},{4,0,3,AXIS_X},{5,0,3,AXIS_X},
	{0,1,3,AXIS_Y},{6,1,3,AXIS_Y},{0,2,3,AXIS_Y},{6,2,3,AXIS_Y},{0,3,3,AXIS_Y},{6,3,3,AXIS_Y},
	{1,4,3,AXIS_X},{2,4,3,AXIS_X},{3,4,3,AXIS_X},{4,4,3,AXIS_X},{5,4,3,AXIS_X},
	{1,1,4,AXIS_Y},{2,1,4,AXIS_X},{3,1,4,AXIS_X},{4,1,4,AXIS_X},{5,1,4,AXIS_Y},
	{1,2,4,AXIS_Y},{2,2,4,AXIS_X},{3,2,4,AXIS_X},{4,2,4,AXIS_X},{5,2,4,AXIS_Y},
	{1,3,4,AXIS_Y},{2,3,4,AXIS_X},{3,3,4,AXIS_X},{4,3,4,AXIS_X},{5,3,4,AXIS_Y},
};

static const FossilBlock skull03[] = {
	{0,0,0,AXIS_Y},{4,0,0,AXIS_Y},
	{0,1,0,AXIS_Y},{1,1,0,AXIS_X},{2,1,0,AXIS_X},{3,1,0,AXIS_X},{4,1,0,AXIS_Y},
	{0,2,0,AXIS_Y},{2,2,0,AXIS_Y},{4,2,0,AXIS_Y},
	{0,3,0,AXIS_X},{1,3,0,AXIS_X},{2,3,0,AXIS_X},{3,3,0,AXIS_X},{4,3,0,AXIS_X},
	{1,0,1,AXIS_X},{2,0,1,AXIS_X},{3,0,1,AXIS_X},
	{0,1,1,AXIS_Z},{4,1,1,AXIS_Z},{0,2,1,AXIS_Z},{4,2,1,AXIS_Z},
	{1,3,1,AXIS_X},{2,3,1,AXIS_X},{3,3,1,AXIS_X},
	{1,0,2,AXIS_X},{2,0,2,AXIS_X},{3,0,2,AXIS_X},
	{0,1,2,AXIS_Z},{4,1,2,AXIS_Z},{0,2,2,AXIS_Z},{4,2,2,AXIS_Z},
	{1,3,2,AXIS_X},{2,3,2,AXIS_X},{3,3,2,AXIS_X},
	{1,0,3,AXIS_X},{2,0,3,AXIS_X},{3,0,3,AXIS_X},
	{0,1,3,AXIS_Z},{4,1,3,AXIS_Z},{0,2,3,AXIS_Z},{4,2,3,AXIS_Z},
	{1,3,3,AXIS_X},{2,3,3,AXIS_X},{3,3,3,AXIS_X},
	{1,0,4,AXIS_X},{2,0,4,AXIS_X},{3,0,4,AXIS_X},
	{0,1,4,AXIS_Y},{1,1,4,AXIS_X},{2,1,4,AXIS_X},{3,1,4,AXIS_X},{4,1,4,AXIS_Y},
	{0,2,4,AXIS_Y},{1,2,4,AXIS_X},{2,2,4,AXIS_X},{3,2,4,AXIS_X},{4,2,4,AXIS_Y},
};

static const FossilBlock skull04[] = {
	{0,0,0,AXIS_Y},{3,0,0,AXIS_Y},
	{0,1,0,AXIS_Y},{1,1,0,AXIS_X},{2,1,0,AXIS_X},{3,1,0,AXIS_Y},
	{0,2,0,AXIS_Y},{3,2,0,AXIS_Y},
	{0,3,0,AXIS_X},{1,3,0,AXIS_X},{2,3,0,AXIS_X},{3,3,0,AXIS_X},
	{1,0,1,AXIS_X},{2,0,1,AXIS_X},
	{0,1,1,AXIS_Y},{3,1,1,AXIS_Y},{0,2,1,AXIS_Y},{3,2,1,AXIS_Y},
	{1,3,1,AXIS_X},{2,3,1,AXIS_X},
	{1,0,2,AXIS_X},{2,0,2,AXIS_X},
	{0,1,2,AXIS_Y},{3,1,2,AXIS_Y},{0,2,2,AXIS_Y},{3,2,2,AXIS_Y},
	{1,3,2,AXIS_X},{2,3,2,AXIS_X},
	{1,1,3,AXIS_Y},{2,1,3,AXIS_Y},{1,2,3,AXIS_Y},{2,2,3,AXIS_Y},
};

static const FossilBlock spine01[] = {
	{1,2,0,AXIS_Z},
	{0,0,1,AXIS_Y},{2,0,1,AXIS_Y},{0,1,1,AXIS_Y},{2,1,1,AXIS_Y},{1,2,1,AXIS_Z},
	{1,2,2,AXIS_Z},
	{0,0,3,AXIS_Y},{2,0,3,AXIS_Y},{0,1,3,AXIS_Y},{2,1,3,AXIS_Y},{1,2,3,AXIS_Z},
	{1,2,4,AXIS_Z},
	{0,0,5,AXIS_Y},{2,0,5,AXIS_Y},{0,1,5,AXIS_Y},{2,1,5,AXIS_Y},{1,2,5,AXIS_Z},
	{1,2,6,AXIS_Z},
	{0,0,7,AXIS_Y},{2,0,7,AXIS_Y},{0,1,7,AXIS_Y},{2,1,7,AXIS_Y},{1,2,7,AXIS_Z},
	{1,2,8,AXIS_Z},
	{0,0,9,AXIS_Y},{2,0,9,AXIS_Y},{0,1,9,AXIS_Y},{2,1,9,AXIS_Y},{1,2,9,AXIS_Z},
	{1,2,10,AXIS_Z},
	{0,0,11,AXIS_Y},{2,0,11,AXIS_Y},{0,1,11,AXIS_Y},{2,1,11,AXIS_Y},{1,2,11,AXIS_Z},
	{1,2,12,AXIS_Z},
};

static const FossilBlock spine02[] = {
	{2,3,0,AXIS_Z},
	{1,0,1,AXIS_X},{3,0,1,AXIS_X},{0,1,1,AXIS_Y},{4,1,1,AXIS_Y},{0,2,1,AXIS_Y},{4,2,1,AXIS_Y},
	{1,3,1,AXIS_X},{2,3,1,AXIS_Z},{3,3,1,AXIS_X},
	{2,3,2,AXIS_Z},
	{1,0,3,AXIS_X},{3,0,3,AXIS_X},{0,1,3,AXIS_Y},{4,1,3,AXIS_Y},{0,2,3,AXIS_Y},{4,2,3,AXIS_Y},
	{1,3,3,AXIS_X},{2,3,3,AXIS_Z},{3,3,3,AXIS_X},
	{2,3,4,AXIS_Z},
	{1,0,5,AXIS_X},{3,0,5,AXIS_X},{0,1,5,AXIS_Y},{4,1,5,AXIS_Y},{0,2,5,AXIS_Y},{4,2,5,AXIS_Y},
	{1,3,5,AXIS_X},{2,3,5,AXIS_Z},{3,3,5,AXIS_X},
	{2,3,6,AXIS_Z},
	{1,0,7,AXIS_X},{3,0,7,AXIS_X},{0,1,7,AXIS_Y},{4,1,7,AXIS_Y},{0,2,7,AXIS_Y},{4,2,7,AXIS_Y},
	{1,3,7,AXIS_X},{2,3,7,AXIS_Z},{3,3,7,AXIS_X},
	{2,3,8,AXIS_Z},
	{1,0,9,AXIS_X},{3,0,9,AXIS_X},{0,1,9,AXIS_Y},{4,1,9,AXIS_Y},{0,2,9,AXIS_Y},{4,2,9,AXIS_Y},
	{1,3,9,AXIS_X},{2,3,9,AXIS_Z},{3,3,9,AXIS_X},
	{2,3,10,AXIS_Z},
	{1,0,11,AXIS_X},{3,0,11,AXIS_X},{0,1,11,AXIS_Y},{4,1,11,AXIS_Y},{0,2,11,AXIS_Y},{4,2,11,AXIS_Y},
	{1,3,11,AXIS_X},{2,3,11,AXIS_Z},{3,3,11,AXIS_X},
	{2,3,12,AXIS_Z},
};

static const FossilBlock spine03[] = {
	{3,3,0,AXIS_Z},
	{0,0,1,AXIS_Y},{1,0,1,AXIS_X},{5,0,1,AXIS_X},{6,0,1,AXIS_Y},
	{0,1,1,AXIS_Y},{6,1,1,AXIS_Y},{0,2,1,AXIS_Y},{6,2,1,AXIS_Y},
	{0,3,1,AXIS_X},{1,3,1,AXIS_X},{2,3,1,AXIS_X},{3,3,1,AXIS_Z},{4,3,1,AXIS_X},{5,3,1,AXIS_X},{6,3,1,AXIS_X},
	{3,3,2,AXIS_Z},
	{0,0,3,AXIS_Y},{1,0,3,AXIS_X},{5,0,3,AXIS_X},{6,0,3,AXIS_Y},
	{0,1,3,AXIS_Y},{6,1,3,AXIS_Y},{0,2,3,AXIS_Y},{6,2,3,AXIS_Y},
	{0,3,3,AXIS_X},{1,3,3,AXIS_X},{2,3,3,AXIS_X},{3,3,3,AXIS_Z},{4,3,3,AXIS_X},{5,3,3,AXIS_X},{6,3,3,AXIS_X},
	{3,3,4,AXIS_Z},
	{0,0,5,AXIS_Y},{1,0,5,AXIS_X},{5,0,5,AXIS_X},{6,0,5,AXIS_Y},
	{0,1,5,AXIS_Y},{6,1,5,AXIS_Y},{0,2,5,AXIS_Y},{6,2,5,AXIS_Y},
	{0,3,5,AXIS_X},{1,3,5,AXIS_X},{2,3,5,AXIS_X},{3,3,5,AXIS_Z},{4,3,5,AXIS_X},{5,3,5,AXIS_X},{6,3,5,AXIS_X},
	{3,3,6,AXIS_Z},
	{0,0,7,AXIS_Y},{1,0,7,AXIS_X},{5,0,7,AXIS_X},{6,0,7,AXIS_Y},
	{0,1,7,AXIS_Y},{6,1,7,AXIS_Y},{0,2,7,AXIS_Y},{6,2,7,AXIS_Y},
	{0,3,7,AXIS_X},{1,3,7,AXIS_X},{2,3,7,AXIS_X},{3,3,7,AXIS_Z},{4,3,7,AXIS_X},{5,3,7,AXIS_X},{6,3,7,AXIS_X},
	{3,3,8,AXIS_Z},
	{0,0,9,AXIS_Y},{1,0,9,AXIS_X},{5,0,9,AXIS_X},{6,0,9,AXIS_Y},
	{0,1,9,AXIS_Y},{6,1,9,AXIS_Y},{0,2,9,AXIS_Y},{6,2,9,AXIS_Y},
	{0,3,9,AXIS_X},{1,3,9,AXIS_X},{2,3,9,AXIS_X},{3,3,9,AXIS_Z},{4,3,9,AXIS_X},{5,3,9,AXIS_X},{6,3,9,AXIS_X},
	{3,3,10,AXIS_Z},
	{0,0,11,AXIS_Y},{1,0,11,AXIS_X},{5,0,11,AXIS_X},{6,0,11,AXIS_Y},
	{0,1,11,AXIS_Y},{6,1,11,AXIS_Y},{0,2,11,AXIS_Y},{6,2,11,AXIS_Y},
	{0,3,11,AXIS_X},{1,3,11,AXIS_X},{2,3,11,AXIS_X},{3,3,11,AXIS_Z},{4,3,11,AXIS_X},{5,3,11,AXIS_X},{6,3,11,AXIS_X},
	{3,3,12,AXIS_Z},
};

static const FossilBlock spine04[] = {
	{4,4,0,AXIS_Z},
	{1,0,1,AXIS_X},{2,0,1,AXIS_X},{6,0,1,AXIS_X},{7,0,1,AXIS_X},
	{0,1,1,AXIS_Y},{8,1,1,AXIS_Y},{0,2,1,AXIS_Y},{8,2,1,AXIS_Y},{0,3,1,AXIS_Y},{8,3,1,AXIS_Y},
	{0,4,1,AXIS_X},{1,4,1,AXIS_X},{2,4,1,AXIS_X},{3,4,1,AXIS_X},{4,4,1,AXIS_Z},{5,4,1,AXIS_X},{6,4,1,AXIS_X},{7,4,1,AXIS_X},{8,4,1,AXIS_X},
	{4,4,2,AXIS_Z},
	{1,0,3,AXIS_X},{2,0,3,AXIS_X},{6,0,3,AXIS_X},{7,0,3,AXIS_X},
	{0,1,3,AXIS_Y},{8,1,3,AXIS_Y},{0,2,3,AXIS_Y},{8,2,3,AXIS_Y},{0,3,3,AXIS_Y},{8,3,3,AXIS_Y},
	{0,4,3,AXIS_X},{1,4,3,AXIS_X},{2,4,3,AXIS_X},{3,4,3,AXIS_X},{4,4,3,AXIS_Z},{5,4,3,AXIS_X},{6,4,3,AXIS_X},{7,4,3,AXIS_X},{8,4,3,AXIS_X},
	{4,4,4,AXIS_Z},
	{1,0,5,AXIS_X},{2,0,5,AXIS_X},{6,0,5,AXIS_X},{7,0,5,AXIS_X},
	{0,1,5,AXIS_Y},{8,1,5,AXIS_Y},{0,2,5,AXIS_Y},{8,2,5,AXIS_Y},{0,3,5,AXIS_Y},{8,3,5,AXIS_Y},
	{0,4,5,AXIS_X},{1,4,5,AXIS_X},{2,4,5,AXIS_X},{3,4,5,AXIS_X},{4,4,5,AXIS_Z},{5,4,5,AXIS_X},{6,4,5,AXIS_X},{7,4,5,AXIS_X},{8,4,5,AXIS_X},
	{4,4,6,AXIS_Z},
	{1,0,7,AXIS_X},{2,0,7,AXIS_X},{6,0,7,AXIS_X},{7,0,7,AXIS_X},
	{0,1,7,AXIS_Y},{8,1,7,AXIS_Y},{0,2,7,AXIS_Y},{8,2,7,AXIS_Y},{0,3,7,AXIS_Y},{8,3,7,AXIS_Y},
	{0,4,7,AXIS_X},{1,4,7,AXIS_X},{2,4,7,AXIS_X},{3,4,7,AXIS_X},{4,4,7,AXIS_Z},{5,4,7,AXIS_X},{6,4,7,AXIS_X},{7,4,7,AXIS_X},{8,4,7,AXIS_X},
	{4,4,8,AXIS_Z},
	{1,0,9,AXIS_X},{2,0,9,AXIS_X},{6,0,9,AXIS_X},{7,0,9,AXIS_X},
	{0,1,9,AXIS_Y},{8,1,9,AXIS_Y},{0,2,9,AXIS_Y},{8,2,9,AXIS_Y},{0,3,9,AXIS_Y},{8,3,9,AXIS_Y},
	{0,4,9,AXIS_X},{1,4,9,AXIS_X},{2,4,9,AXIS_X},{3,4,9,AXIS_X},{4,4,9,AXIS_Z},{5,4,9,AXIS_X},{6,4,9,AXIS_X},{7,4,9,AXIS_X},{8,4,9,AXIS_X},
	{4,4,10,AXIS_Z},
	{1,0,11,AXIS_X},{2,0,11,AXIS_X},{6,0,11,AXIS_X},{7,0,11,AXIS_X},
	{0,1,11,AXIS_Y},{8,1,11,AXIS_Y},{0,2,11,AXIS_Y},{8,2,11,AXIS_Y},{0,3,11,AXIS_Y},{8,3,11,AXIS_Y},
	{0,4,11,AXIS_X},{1,4,11,AXIS_X},{2,4,11,AXIS_X},{3,4,11,AXIS_X},{4,4,11,AXIS_Z},{5,4,11,AXIS_X},{6,4,11,AXIS_X},{7,4,11,AXIS_X},{8,4,11,AXIS_X},
	{4,4,12,AXIS_Z},
};

static const FossilStructure skullStructures[4] = {
	{6, 5, 7, skull01, (int)(sizeof(skull01)/sizeof(skull01[0]))},
	{7, 5, 5, skull02, (int)(sizeof(skull02)/sizeof(skull02[0]))},
	{5, 4, 5, skull03, (int)(sizeof(skull03)/sizeof(skull03[0]))},
	{4, 4, 4, skull04, (int)(sizeof(skull04)/sizeof(skull04[0]))},
};

static const FossilStructure spineStructures[4] = {
	{3, 3, 13, spine01, (int)(sizeof(spine01)/sizeof(spine01[0]))},
	{5, 4, 13, spine02, (int)(sizeof(spine02)/sizeof(spine02[0]))},
	{7, 4, 13, spine03, (int)(sizeof(spine03)/sizeof(spine03[0]))},
	{9, 5, 13, spine04, (int)(sizeof(spine04)/sizeof(spine04[0]))},
};

bool FossilFeature::place(Level *level, Random *random, int x, int y, int z)
{
	bool useSpine = random->nextInt(2) == 0;
	int variant   = random->nextInt(4);

	const FossilStructure &s = useSpine ? spineStructures[variant] : skullStructures[variant];

	int ox = x - s.width  / 2;
	int oz = z - s.length / 2;

	int surface = Level::maxBuildHeight;
	for (int dx = 0; dx < s.width; dx++)
		for (int dz = 0; dz < s.length; dz++)
		{
			int h = level->getHeightmap(ox + dx, oz + dz);
			if (h > 0 && h < surface) surface = h;
		}
	if (surface >= Level::maxBuildHeight)
		surface = level->getSeaLevel();

	int oy = surface - 15 - random->nextInt(10);
	if (oy < 10) oy = 10;

	for (int i = 0; i < s.blockCount; i++)
	{
		const FossilBlock &b = s.blocks[i];
		int bx = ox + b.x;
		int by = oy + b.y;
		int bz = oz + b.z;

		if (by < 1 || by >= Level::maxBuildHeight)
			continue;

		if (level->getMaterial(bx, by, bz)->isLiquid())
			continue;

		int roll = random->nextInt(10);
		if (roll == 1)
			continue;
		else if (roll == 0)
			level->setTileAndData(bx, by, bz, Tile::coal_ore_Id, 0, Tile::UPDATE_CLIENTS);
		else
			level->setTileAndData(bx, by, bz, Tile::bone_block_Id, b.data, Tile::UPDATE_CLIENTS);
	}

	return true;
}
