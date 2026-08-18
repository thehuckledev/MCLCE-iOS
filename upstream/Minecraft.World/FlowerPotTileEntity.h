#pragma once
#include "TileEntity.h"

class FlowerPotTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_FLOWERPOTTILEENTITY; }
	static TileEntity *create() { return new FlowerPotTileEntity(); }

private:
	int flowerItemId;
	int flowerAuxValue;

public:
	FlowerPotTileEntity();
	void setFlowerItem(int itemId, int auxValue);
	int getFlowerItemId() const;
	int getFlowerAuxValue() const;
	bool hasFlower() const;

	void save(CompoundTag *tag);
	void load(CompoundTag *tag);
	shared_ptr<Packet> getUpdatePacket();
	virtual shared_ptr<TileEntity> clone();
};
