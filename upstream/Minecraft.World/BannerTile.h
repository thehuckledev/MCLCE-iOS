#pragma once

#include "BaseEntityTile.h"
#include "TileEntity.h"

#include "stdafx.h"
#include "Material.h"

class Player;

class BannerTile : public BaseEntityTile
{
	friend class Tile;
private:
	bool onGround;
	int m_dropColor;

protected:
	BannerTile(int id, bool onGround);

public:
	Icon *getTexture(int face, int data);
	virtual void updateDefaultShape();
	AABB *getAABB(Level *level, int x, int y, int z);
	AABB *getTileAABB(Level *level, int x, int y, int z);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	int getRenderShape();
	bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	bool isSolidRender(bool isServerLevel = false);

protected:
	shared_ptr<TileEntity> newTileEntity(Level *level);

public:
	int getResource(int data, Random *random, int playerBonusLevel);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);

	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player) override;
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel) override;
	using BaseEntityTile::spawnResources;
};
