#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "Biome.h"
#include "Item.h"
#include "ItemInstance.h"
#include "LootTableManager.h"
#include "WeighedTreasure.h"
#include "Villager.h"
#include "Zombie.h"
#include "IglooFeature.h"

bool IglooFeature::place(Level *level, Random *random, int x, int y, int z)
{
	while (y > 2 && level->isEmptyTile(x, y - 1, z))
		y--;

	Biome *biome = level->getBiome(x, z);
	if (biome != Biome::iceFlats && biome != Biome::iceMountains &&
		biome != Biome::coldTaiga && biome != Biome::coldTaigaHills)
		return false;

	for (int dx = -3; dx <= 3; dx++)
		for (int dz = -3; dz <= 3; dz++)
			if (abs(dx) + abs(dz) < 6)
				level->setTileAndData(x + dx, y, z + dz, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	for (int dy = 1; dy <= 2; dy++)
	{
		for (int dx = -3; dx <= 3; dx++)
		{
			for (int dz = -3; dz <= 3; dz++)
			{
				bool onPerimeter = (abs(dx) == 3 || abs(dz) == 3) && abs(dx) + abs(dz) < 6;
				if (!onPerimeter)
					continue;
				if (dx == 0 && dz == 3)
					continue;
				if ((dx == 3 || dx == -3) && dz == 0 && dy == 1)
					level->setTileAndData(x + dx, y + dy, z + dz, Tile::ice_Id, 0, Tile::UPDATE_CLIENTS);
				else
					level->setTileAndData(x + dx, y + dy, z + dz, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	level->setTileAndData(x, y + 3, z + 3, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 1, y,     z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x,     y,     z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y,     z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 1, y + 1, z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y + 1, z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 1, y + 2, z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y + 2, z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y + 3, z + 4, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	for (int dx = -2; dx <= 2; dx++)
		for (int dz = -2; dz <= 2; dz++)
			if (abs(dx) == 2 || abs(dz) == 2)
				level->setTileAndData(x + dx, y + 3, z + dz, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			level->setTileAndData(x + dx, y + 4, z + dz, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	for (int dy = 1; dy <= 2; dy++)
		for (int dx = -2; dx <= 2; dx++)
			for (int dz = -2; dz <= 2; dz++)
				level->setTileAndData(x + dx, y + dy, z + dz, 0, 0, Tile::UPDATE_CLIENTS);

	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			level->setTileAndData(x + dx, y + 3, z + dz, 0, 0, Tile::UPDATE_CLIENTS);

	for (int dx = -1; dx <= 1; dx++)
		for (int dz = -1; dz <= 1; dz++)
			level->setTileAndData(x + dx, y + 1, z + dz, Tile::carpet_Id, 0, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 1, y + 1, z - 2, Tile::carpet_Id, 8, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x,     y + 1, z - 2, Tile::carpet_Id, 8, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, y + 1, z - 2, Tile::carpet_Id, 8, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x + 2, y + 1, z + 1, Tile::furnace_Id, 4, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 1, z,     Tile::redstone_torch_Id, 5, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 1, z - 1, Tile::crafting_table_Id, 0, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 2, y + 1, z,     Tile::bed_Id, 2, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, y + 1, z - 1, Tile::bed_Id, 2 | 8, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 2, y + 1, z - 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 1, z - 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, y + 1, z + 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 1, z + 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, y + 2, z - 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 2, z - 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, y + 2, z + 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, y + 2, z + 2, Tile::snow_Id, 0, Tile::UPDATE_CLIENTS);

	if (random->nextInt(2) == 0)
		placeBasement(level, random, x, y, z);

	return true;
}

void IglooFeature::placeBasement(Level *level, Random *random, int x, int y, int z)
{
	int by = y - 9;
	int rz = z + 2;

	for (int dx = -3; dx <= 3; dx++)
	{
		for (int dz = -4; dz <= 4; dz++)
		{
			for (int dy = 0; dy <= 4; dy++)
			{
				bool isWall = abs(dx) == 3 || abs(dz) == 4 || dy == 0 || dy == 4;
				if (isWall)
				{
					int data = 0;
					if (random->nextInt(5) == 0)
						data = 2;
					else if (random->nextInt(4) == 0)
						data = 1;
					level->setTileAndData(x + dx, by + dy, rz + dz, Tile::stonebrick_Id, data, Tile::UPDATE_CLIENTS);
				}
				else
				{
					level->setTileAndData(x + dx, by + dy, rz + dz, 0, 0, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}

	level->setTileAndData(x, y, z - 1, Tile::trapdoor_Id, 4, Tile::UPDATE_CLIENTS);

	for (int i = 1; i <= 5; i++)
		level->setTileAndData(x, y - i, z - 1, Tile::ladder_Id, 3, Tile::UPDATE_CLIENTS);

	for (int dy = 1; dy <= 3; dy++)
		level->setTileAndData(x, by + dy, z - 1, Tile::ladder_Id, 3, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x + 2, by + 1, rz - 2, Tile::chest_Id, 4, Tile::UPDATE_CLIENTS);
	auto chestEntity = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x + 2, by + 1, rz - 2));
	if (chestEntity)
	{
		std::vector<LootTableDropResult> drops = LootTableManager::Get().ResolveDrops(
			"chests/igloo_chest", false, 0,
			[random](int bound) { return random->nextInt(bound); });
		WeighedTreasure::addChestItems(random, drops, chestEntity);
		chestEntity->setItem(0, make_shared<ItemInstance>(Item::golden_apple_Id, 1, 0));
	}

	level->setTileAndData(x - 2, by + 2, rz - 2, Tile::brewing_stand_Id, 0, Tile::UPDATE_CLIENTS);
	auto brewEntity = dynamic_pointer_cast<BrewingStandTileEntity>(level->getTileEntity(x - 2, by + 2, rz - 2));
	if (brewEntity)
		brewEntity->setItem(1, make_shared<ItemInstance>(Item::potion_Id, 1, 16392));
	level->setTileAndData(x - 2, by + 1, rz,     Tile::cauldron_Id, 2, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 2, by + 1, rz - 3, Tile::spruce_stairs_Id, 7, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, by + 1, rz - 2, Tile::wooden_slab_Id,   9, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x - 2, by + 1, rz - 1, Tile::spruce_stairs_Id, 6, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x + 2, by + 1, rz - 1, Tile::carpet_Id, 14, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, by + 1, rz,     Tile::carpet_Id, 14, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 2, by + 1, rz + 1, Tile::carpet_Id, 14, Tile::UPDATE_CLIENTS);

	for (int cdx = -2; cdx <= 2; cdx++)
	{
		bool isBar = (cdx % 2 != 0);
		level->setTileAndData(x + cdx, by + 1, rz + 2, isBar ? Tile::iron_bars_Id : Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x + cdx, by + 2, rz + 2, isBar ? Tile::iron_bars_Id : Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x + cdx, by + 3, rz + 2, Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
	}

	for (int dy = 1; dy <= 3; dy++)
	{
		level->setTileAndData(x - 2, by + dy, rz + 3, Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x,     by + dy, rz + 3, Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
		level->setTileAndData(x + 2, by + dy, rz + 3, Tile::stonebrick_Id, 0, Tile::UPDATE_CLIENTS);
	}

	level->setTileAndData(x + 2, by + 3, rz + 1, Tile::web_Id, 0, Tile::UPDATE_CLIENTS);

	level->setTileAndData(x - 1, by + 3, z - 1, Tile::torch_Id, 3, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x + 1, by + 3, z - 1, Tile::torch_Id, 3, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, by + 3, rz + 1, Tile::torch_Id, 4, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, by + 2, rz + 1, Tile::wall_standing_sign_Id, 2, Tile::UPDATE_CLIENTS);
	auto signEntity = dynamic_pointer_cast<SignTileEntity>(level->getTileEntity(x, by + 2, rz + 1));
	if (signEntity)
	{
		wstring empty = L"";
		wstring line1 = L"<----";
		wstring line2 = L"---->";
		signEntity->SetMessage(0, empty);
		signEntity->SetMessage(1, line1);
		signEntity->SetMessage(2, line2);
		signEntity->SetMessage(3, empty);
	}
	level->setTileAndData(x - 2, by + 2, rz - 3, Tile::flower_pot_Id, 9, Tile::UPDATE_CLIENTS);

	auto zombieVillager = std::make_shared<Zombie>(level);
	zombieVillager->setVillager(true);
	zombieVillager->setPersistenceRequired();
	zombieVillager->moveTo(x - 1 + 0.5, by + 1, rz + 3 + 0.5, 0, 0);
	level->addEntity(zombieVillager);

	auto villager = std::make_shared<Villager>(level);
	villager->moveTo(x + 1 + 0.5, by + 1, rz + 3 + 0.5, 0, 0);
	level->addEntity(villager);
}
