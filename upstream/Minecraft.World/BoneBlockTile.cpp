#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.player.h"
#include "IconRegister.h"
#include "BoneBlockTile.h"

BoneBlockTile::BoneBlockTile(int id) : HayBlockTile(id)
{
	material = Material::stone;
}

void BoneBlockTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	if (player->canDestroy(this))
		Tile::playerDestroy(level, player, x, y, z, data);
}

void BoneBlockTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = iconRegister->registerIcon(L"bone_block_top");
	icon    = iconRegister->registerIcon(L"bone_block_side");
}
