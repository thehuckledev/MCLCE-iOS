#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "WeighedTreasure.h"
#include "LootTableManager.h"
#include "MonsterRoomFeature.h"

bool MonsterRoomFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int hr = 3;
	int xr = random->nextInt(2) + 2;
	int zr = random->nextInt(2) + 2;

	int holeCount = 0;
	for (int xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int yy = y - 1; yy <= y + hr + 1; yy++)
		{
			for (int zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{
				Material *m = level->getMaterial(xx, yy, zz);
				if (yy == y - 1 && !m->isSolid()) return false;
				if (yy == y + hr + 1 && !m->isSolid()) return false;

				if (xx == x - xr - 1 || xx == x + xr + 1 || zz == z - zr - 1 || zz == z + zr + 1)
				{
					if (yy == y && level->isEmptyTile(xx, yy, zz) && level->isEmptyTile(xx, yy + 1, zz))
					{
						holeCount++;
					}
				}

			}
		}
	}

	if (holeCount < 1 || holeCount > 5) return false;

	for (int xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int yy = y + hr; yy >= y - 1; yy--)
		{
			for (int zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{

				if (xx == x - xr - 1 || yy == y - 1 || zz == z - zr - 1 || xx == x + xr + 1 || yy == y + hr + 1 || zz == z + zr + 1)
				{
					if (yy >= 0 && !level->getMaterial(xx, yy - 1, zz)->isSolid())
					{
						level->removeTile(xx, yy, zz);
					}
					else if (level->getMaterial(xx, yy, zz)->isSolid())
					{
						if (yy == y - 1 && random->nextInt(4) != 0)
						{
							level->setTileAndData(xx, yy, zz, Tile::mossy_cobblestone_Id, 0, Tile::UPDATE_CLIENTS);
						}
						else
						{
							level->setTileAndData(xx, yy, zz, Tile::cobblestone_Id, 0, Tile::UPDATE_CLIENTS);
						}
					}
				} else
				{
					level->removeTile(xx, yy, zz);
				}
			}
		}
	}

	for (int cc = 0; cc < 2; cc++)
	{
		for (int i = 0; i < 3; i++)
		{
			int xc = x + random->nextInt(xr * 2 + 1) - xr;
			int yc = y;
			int zc = z + random->nextInt(zr * 2 + 1) - zr;
			if (!level->isEmptyTile(xc, yc, zc)) continue;

			int count = 0;
			if (level->getMaterial(xc - 1, yc, zc)->isSolid()) count++;
			if (level->getMaterial(xc + 1, yc, zc)->isSolid()) count++;
			if (level->getMaterial(xc, yc, zc - 1)->isSolid()) count++;
			if (level->getMaterial(xc, yc, zc + 1)->isSolid()) count++;

			if (count != 1) continue;

			level->setTileAndData(xc, yc, zc, Tile::chest_Id, 0, Tile::UPDATE_CLIENTS);
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity >( level->getTileEntity(xc, yc, zc) );
			if (chest != nullptr )
			{
				WeighedTreasure::addChestItems(random,
					LootTableManager::Get().ResolveDrops("chests/simple_dungeon", false, 0,
						[random](int bound) { return random->nextInt(bound); }),
					chest);
			}

			break;
		}
	}


	level->setTileAndData(x, y, z, Tile::mob_spawner_Id, 0, Tile::UPDATE_CLIENTS);
	shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x, y, z) );
	if( entity != nullptr )
	{
		entity->getSpawner()->setEntityId(randomEntityId(random));
	}

	return true;

}

wstring MonsterRoomFeature::randomEntityId(Random *random)
{
	int id = random->nextInt(4);
	if (id == 0) return wstring(L"Skeleton");
	if (id == 1) return wstring(L"Zombie");
	if (id == 2) return wstring(L"Zombie");
	if (id == 3) return wstring(L"Spider");
	return wstring(L"");
}