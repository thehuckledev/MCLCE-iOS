#pragma once
#include "Tile.h"

class GrassPathTile : public Tile
{
	Icon *iconTop;
	Icon *iconBottom;

public:
	GrassPathTile(int id);

	virtual void updateDefaultShape() override;
	virtual bool isSolidRender(bool isServerLevel = false) override;
	virtual bool isCubeShaped() override;
    virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int neighborId) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual Icon *getTexture(int face, int data) override;
	virtual int getResource(int data, Random *random, int playerBonusLevel) override;
	virtual int getResourceCount(Random *random) override;

private:
	void checkAndConvert(Level *level, int x, int y, int z);
};
