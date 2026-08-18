#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "Material.h"
#include "ItemInstance.h"
#include "Random.h"
#include "BannerTileEntity.h"
#include "BannerTile.h"

BannerTile::BannerTile(int id, bool onGround) : BaseEntityTile(id, Material::wood, isSolidRender())
{
	this->onGround = onGround;
	this->m_dropColor = 15;
	updateDefaultShape();
}

Icon *BannerTile::getTexture(int face, int data)
{
	return Tile::wood->getTexture(face);
}

void BannerTile::updateDefaultShape()
{
	if (onGround)
	{
		float r = 4 / 16.0f;
		float h = 16 / 16.0f;
		this->setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
	}
	else
	{
		this->setShape(0, 0, 0, 1, 1, 1);
	}
}

AABB *BannerTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}

AABB *BannerTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return BaseEntityTile::getTileAABB(level, x, y, z);
}

void BannerTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	shared_ptr<BannerTileEntity> bte = forceEntity
		? dynamic_pointer_cast<BannerTileEntity>(forceEntity)
		: dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
	if (!bte || !bte->isWall()) return;

	int face = level->getData(x, y, z);

	float h0 = 0 / 16.0f;
	float h1 = 16 / 16.0f;
	float d0 = 2 / 16.0f;
	float w0 = 2 / 16.0f;
	float w1 = 14 / 16.0f;

	setShape(0, 0, 0, 1, 1, 1);
	if (face == 2) setShape(w0, h0, 1 - d0, w1, h1, 1);
	if (face == 3) setShape(w0, h0, 0, w1, h1, d0);
	if (face == 4) setShape(1 - d0, h0, w0, 1, h1, w1);
	if (face == 5) setShape(0, h0, w0, d0, h1, w1);
}

int BannerTile::getRenderShape()
{
	return Tile::SHAPE_INVISIBLE;
}

bool BannerTile::isCubeShaped()
{
	return false;
}

bool BannerTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return true;
}

bool BannerTile::isSolidRender(bool isServerLevel)
{
	return false;
}

shared_ptr<TileEntity> BannerTile::newTileEntity(Level *level)
{
	return std::make_shared<BannerTileEntity>();
}

int BannerTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::standing_banner_Id;
}

void BannerTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	shared_ptr<BannerTileEntity> bte = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
	bool isWall = bte && bte->isWall();
	bool remove = false;

	if (!isWall)
	{
		if (!level->getMaterial(x, y - 1, z)->isSolid()) remove = true;
	}
	else
	{
		int face = level->getData(x, y, z);
		remove = true;
		if (face == 2 && level->getMaterial(x, y, z + 1)->isSolid()) remove = false;
		if (face == 3 && level->getMaterial(x, y, z - 1)->isSolid()) remove = false;
		if (face == 4 && level->getMaterial(x + 1, y, z)->isSolid()) remove = false;
		if (face == 5 && level->getMaterial(x - 1, y, z)->isSolid()) remove = false;
	}
	if (remove)
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		level->removeTileEntity(x, y, z);
	}

	BaseEntityTile::neighborChanged(level, x, y, z, type);
}

int BannerTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Tile::standing_banner_Id;
}

void BannerTile::registerIcons(IconRegister *iconRegister)
{
}

void BannerTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	shared_ptr<BannerTileEntity> bte = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
	m_dropColor = (bte != nullptr) ? (15 - (bte->getBaseColor() & 15)) : 0;

	BaseEntityTile::playerWillDestroy(level, x, y, z, data, player);
}

void BannerTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (level->isClientSide) return;

	int color = m_dropColor;
	shared_ptr<BannerTileEntity> bte = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
	if (bte != nullptr) color = 15 - (bte->getBaseColor() & 15);

	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, color));
	}
}
