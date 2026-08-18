#pragma once
#include "HayBlockTile.h"

class BoneBlockTile : public HayBlockTile
{
public:
	BoneBlockTile(int id);
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data) override;
};
