#pragma once

#include "BaseEntityTile.h"

class FlowerPotTile : public BaseEntityTile
{
public:
	static const int TYPE_FLOWER_RED = 1;
	static const int TYPE_FLOWER_YELLOW = 2;
	static const int TYPE_SAPLING_DEFAULT = 3;
	static const int TYPE_SAPLING_EVERGREEN = 4;
	static const int TYPE_SAPLING_BIRCH = 5;
	static const int TYPE_SAPLING_JUNGLE = 6;
	static const int TYPE_MUSHROOM_RED = 7;
	static const int TYPE_MUSHROOM_BROWN = 8;
	static const int TYPE_CACTUS = 9;
	static const int TYPE_DEAD_BUSH = 10;
	static const int TYPE_FERN = 11;
	static const int TYPE_FLOWER_BLUE_ORCHID = 12;
	static const int TYPE_FLOWER_ALLIUM = 13;
	static const int TYPE_FLOWER_AZURE_BLUET = 14;
	static const int TYPE_FLOWER_OXEYE_DAISY = 15;

	FlowerPotTile(int id);
	shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual void createBlockStateDefinition() override;
	virtual int defaultBlockState() override;
	virtual int convertBlockStateToLegacyData(BlockState *state) override;
	virtual Tile::BlockState getBlockState(LevelSource *level, int x, int y, int z) override;
	virtual Tile::BlockState getBlockState(int data);

	void updateDefaultShape();
	bool isSolidRender(bool isServerLevel = false);
	int getRenderShape();
	bool isCubeShaped();
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
	int cloneTileId(Level *level, int x, int y, int z);
	int cloneTileData(Level *level, int x, int y, int z);
	bool useOwnCloneData();
	bool mayPlace(Level *level, int x, int y, int z);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	void onRemove(Level *level, int x, int y, int z, int id, int data);
	using Tile::spawnResources;
	void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	int getResource(int data, Random *random, int playerBonusLevel);
	static shared_ptr<ItemInstance> getItemFromType(int type);
	static int getTypeFromItem(shared_ptr<ItemInstance> item);
};