#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "FlowerPotTile.h"
#include "FlowerPotTileEntity.h"

FlowerPotTile::FlowerPotTile(int id) : BaseEntityTile(id, Material::decoration, isSolidRender())
{
	setLightBlock(0);
	updateDefaultShape();
	sendTileData();
}

shared_ptr<TileEntity> FlowerPotTile::newTileEntity(Level *level)
{
	return std::make_shared<FlowerPotTileEntity>();
}

void FlowerPotTile::createBlockStateDefinition()
{
	if (!m_blockStateDefinition)
		m_blockStateDefinition = new BlockStateDefinition(this);
}

int FlowerPotTile::defaultBlockState()
{
	return 0;
}

int FlowerPotTile::convertBlockStateToLegacyData(BlockState *state)
{
	return state ? (state->value & 0xF) : 0;
}

Tile::BlockState FlowerPotTile::getBlockState(int data)
{
	return Tile::BlockState(data & 0xF);
}

Tile::BlockState FlowerPotTile::getBlockState(LevelSource *level, int x, int y, int z)
{
	return Tile::BlockState(level->getData(x, y, z) & 0xF);
}

void FlowerPotTile::updateDefaultShape()
{
	float size = 6.0f / 16.0f;
	float half = size / 2;
	setShape(0.5f - half, 0, 0.5f - half, 0.5f + half, size, 0.5f + half);
}

bool FlowerPotTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int FlowerPotTile::getRenderShape()
{
	return SHAPE_FLOWER_POT;
}

bool FlowerPotTile::isCubeShaped()
{
	return false;
}

bool FlowerPotTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item == nullptr) return false;
	if (level->getData(x, y, z) != 0) return false;
	int type = getTypeFromItem(item);

	if (type > 0)
	{
		level->setData(x, y, z, type, Tile::UPDATE_CLIENTS);

		shared_ptr<TileEntity> teBase = level->getTileEntity(x, y, z);
		shared_ptr<FlowerPotTileEntity> te = teBase ? std::dynamic_pointer_cast<FlowerPotTileEntity>(teBase) : nullptr;
		if (te == nullptr)
		{
			te = std::make_shared<FlowerPotTileEntity>();
			level->setTileEntity(x, y, z, te);
		}
		te->setFlowerItem(item->getItem()->id, item->getAuxValue());

		if (!player->abilities.instabuild)
		{
			if (--item->count <= 0)
			{
				player->inventory->setItem(player->inventory->selected, nullptr);
			}
		}

		return true;
	}

	return false;
}

int FlowerPotTile::cloneTileId(Level *level, int x, int y, int z)
{
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<FlowerPotTileEntity> fpte = te ? std::dynamic_pointer_cast<FlowerPotTileEntity>(te) : nullptr;
	if (fpte && fpte->hasFlower())
		return fpte->getFlowerItemId();

	shared_ptr<ItemInstance> item = getItemFromType(level->getData(x, y, z));
	return item ? item->id : Item::flower_pot_Id;
}

int FlowerPotTile::cloneTileData(Level *level, int x, int y, int z)
{
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<FlowerPotTileEntity> fpte = te ? std::dynamic_pointer_cast<FlowerPotTileEntity>(te) : nullptr;
	if (fpte && fpte->hasFlower())
		return fpte->getFlowerAuxValue();

	shared_ptr<ItemInstance> item = getItemFromType(level->getData(x, y, z));
	return item ? item->getAuxValue() : 0;
}

bool FlowerPotTile::useOwnCloneData()
{
	return true;
}

bool FlowerPotTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && level->isTopSolidBlocking(x, y - 1, z);
}

void FlowerPotTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isTopSolidBlocking(x, y - 1, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);

		level->removeTile(x, y, z);
	}
}

void FlowerPotTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (data > 0)
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<FlowerPotTileEntity> fpte = te ? std::dynamic_pointer_cast<FlowerPotTileEntity>(te) : nullptr;
		shared_ptr<ItemInstance> item;
		if (fpte && fpte->hasFlower())
			item = std::make_shared<ItemInstance>(fpte->getFlowerItemId(), 1, fpte->getFlowerAuxValue());
		else
			item = getItemFromType(data);
		if (item != nullptr)
			popResource(level, x, y, z, item);
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

void FlowerPotTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	Tile::spawnResources(level, x, y, z, data, odds, playerBonusLevel);
}

int FlowerPotTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::flower_pot_Id;
}

shared_ptr<ItemInstance> FlowerPotTile::getItemFromType(int type)
{
	switch (type)
	{
	case TYPE_FLOWER_RED:
		return std::make_shared<ItemInstance>(Tile::rose, 1, 0);
	case TYPE_FLOWER_BLUE_ORCHID:
		return std::make_shared<ItemInstance>(Tile::rose, 1, Rose::BLUE_ORCHID);
	case TYPE_FLOWER_ALLIUM:
		return std::make_shared<ItemInstance>(Tile::rose, 1, Rose::ALLIUM);
	case TYPE_FLOWER_AZURE_BLUET:
		return std::make_shared<ItemInstance>(Tile::rose, 1, Rose::AZURE_BLUET);
	case TYPE_FLOWER_OXEYE_DAISY:
		return std::make_shared<ItemInstance>(Tile::rose, 1, Rose::OXEYE_DAISY);
	case TYPE_FLOWER_YELLOW:
		return std::make_shared<ItemInstance>(Tile::flower);
	case TYPE_CACTUS:
		return std::make_shared<ItemInstance>(Tile::cactus);
	case TYPE_MUSHROOM_BROWN:
		return std::make_shared<ItemInstance>(Tile::mushroom_brown);
	case TYPE_MUSHROOM_RED:
		return std::make_shared<ItemInstance>(Tile::mushroom_red);
	case TYPE_DEAD_BUSH:
		return std::make_shared<ItemInstance>(Tile::deadBush);
	case TYPE_SAPLING_DEFAULT:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_DEFAULT);
	case TYPE_SAPLING_BIRCH:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_BIRCH);
	case TYPE_SAPLING_EVERGREEN:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_EVERGREEN);
	case TYPE_SAPLING_JUNGLE:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_JUNGLE);
	case TYPE_FERN:
		return std::make_shared<ItemInstance>(Tile::tallgrass, 1, TallGrass::FERN);
	}

	return nullptr;
}

int FlowerPotTile::getTypeFromItem(shared_ptr<ItemInstance> item)
{
	int id = item->getItem()->id;

	if (id == Tile::red_flower_Id) return TYPE_FLOWER_RED;
	if (id == Tile::yellow_flower_Id) return TYPE_FLOWER_YELLOW;
	if (id == Tile::cactus_Id) return TYPE_CACTUS;
	if (id == Tile::mushroom_brown_Id) return TYPE_MUSHROOM_BROWN;
	if (id == Tile::mushroom_red_Id) return TYPE_MUSHROOM_RED;
	if (id == Tile::deadbush_Id) return TYPE_DEAD_BUSH;

	if (id == Tile::sapling_Id)
	{
		switch (item->getAuxValue())
		{
		case Sapling::TYPE_DEFAULT:
			return TYPE_SAPLING_DEFAULT;
		case Sapling::TYPE_BIRCH:
			return TYPE_SAPLING_BIRCH;
		case Sapling::TYPE_EVERGREEN:
			return TYPE_SAPLING_EVERGREEN;
		case Sapling::TYPE_JUNGLE:
			return TYPE_SAPLING_JUNGLE;
		}
	}

	if (id == Tile::tallgrass_Id)
	{
		switch (item->getAuxValue())
		{
		case TallGrass::FERN:
			return TYPE_FERN;
		}
	}

	return 0;
}
