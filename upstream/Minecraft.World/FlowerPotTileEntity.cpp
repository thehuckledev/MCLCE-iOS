#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "FlowerPotTileEntity.h"
#include "TileEntityDataPacket.h"

FlowerPotTileEntity::FlowerPotTileEntity() : flowerItemId(-1), flowerAuxValue(0)
{
}

void FlowerPotTileEntity::setFlowerItem(int itemId, int auxValue)
{
	flowerItemId = itemId;
	flowerAuxValue = auxValue;
}

int FlowerPotTileEntity::getFlowerItemId() const
{
	return flowerItemId;
}

int FlowerPotTileEntity::getFlowerAuxValue() const
{
	return flowerAuxValue;
}

bool FlowerPotTileEntity::hasFlower() const
{
	return flowerItemId >= 0;
}

void FlowerPotTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putInt(L"Item", flowerItemId);
	tag->putInt(L"Data", flowerAuxValue);
}

void FlowerPotTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	flowerItemId = tag->getInt(L"Item");
	flowerAuxValue = tag->getInt(L"Data");
}

shared_ptr<Packet> FlowerPotTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_FLOWER_POT, tag);
}

shared_ptr<TileEntity> FlowerPotTileEntity::clone()
{
	shared_ptr<FlowerPotTileEntity> e = std::make_shared<FlowerPotTileEntity>();
	TileEntity::clone(e);
	e->flowerItemId = flowerItemId;
	e->flowerAuxValue = flowerAuxValue;
	return e;
}
