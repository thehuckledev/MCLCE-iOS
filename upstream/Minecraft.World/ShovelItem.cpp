#include "stdafx.h"

#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "ShovelItem.h"

TileArray *ShovelItem::diggables = nullptr;

void ShovelItem::staticCtor()
{
	ShovelItem::diggables = new TileArray( SHOVEL_DIGGABLES);
	diggables->data[0] = Tile::grass;
	diggables->data[1] = Tile::dirt;
	diggables->data[2] = Tile::sand;
	diggables->data[3] = Tile::gravel;
	diggables->data[4] = Tile::topSnow;
	diggables->data[5] = Tile::snow;
	diggables->data[6] = Tile::clay;
	diggables->data[7] = Tile::farmland;
	diggables->data[8] = Tile::soulsand;
	diggables->data[9] = Tile::mycel;
	diggables->data[10] = Tile::grass_path;
}

ShovelItem::ShovelItem(int id, const Tier *tier) : DiggerItem(id, 1, tier, diggables)
{
}

bool ShovelItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	int targetType = level->getTile(x, y, z);
	int above = level->getTile(x, y + 1, z);

	if (face != 0 && above == 0 && (targetType == Tile::grass_Id || targetType == Tile::dirt_Id))
	{
		if (!bTestUseOnOnly)
		{
			Tile *tile = Tile::grass_path;
			level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);

			if (level->isClientSide) return true;
			level->setTileAndUpdate(x, y, z, tile->id);
			instance->hurtAndBreak(1, player);
		}
		return true;
	}

	return false;
}

bool ShovelItem::canDestroySpecial(Tile *tile)
{
	if (tile == Tile::topSnow) return true;
	if (tile == Tile::snow) return true;
	return false;
}