#include "stdafx.h"
#include "HalfSlabTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "Facing.h"

HalfSlabTile::HalfSlabTile(int id, Material *material)
    : Tile(id, material, true)
{
    
    Tile::lightBlock[id] = 0xFF;
}

void HalfSlabTile::createBlockStateDefinition()
{
    if (!m_blockStateDefinition)
        m_blockStateDefinition = new BlockStateDefinition(this);
}

int HalfSlabTile::defaultBlockState()
{
    return 0;
}

int HalfSlabTile::convertBlockStateToLegacyData(BlockState *state)
{
    return state ? (state->value & (TYPE_MASK | TOP_SLOT_BIT)) : 0;
}

Tile::BlockState HalfSlabTile::getBlockState(int data)
{
    return Tile::BlockState(data & (TYPE_MASK | TOP_SLOT_BIT));
}

Tile::BlockState HalfSlabTile::getBlockState(LevelSource *level, int x, int y, int z)
{
    return Tile::BlockState(level->getData(x, y, z) & (TYPE_MASK | TOP_SLOT_BIT));
}

void HalfSlabTile::DerivedInit()
{
    if (!isFullSize())
    {
        setLightBlock(0);
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
    }
    else
    {
        Tile::solid[id] = true;
    }
}

void HalfSlabTile::updateDefaultShape()
{
    
    if (isFullSize())
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    else
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
}

void HalfSlabTile::updateShape(
    LevelSource *level, int x, int y, int z,
    int forceData, shared_ptr<TileEntity> forceEntity)
{
   
    if (isFullSize())
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    int data = (forceData >= 0) ? forceData : level->getData(x, y, z);
    bool isUpper = (data & TOP_SLOT_BIT) != 0;

    if (isUpper)
        setShape(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
    else
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
}

void HalfSlabTile::addAABBs(
    Level *level, int x, int y, int z,
    AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
    
    updateShape(level, x, y, z);
    Tile::addAABBs(level, x, y, z, box, boxes, source);
}

bool HalfSlabTile::isSolidRender(bool isServerLevel)
{
   
    return isFullSize() != 0;
}

bool HalfSlabTile::isSilkTouchable()
{
    
    return false;
}

int HalfSlabTile::getResourceCount(Random *random)
{
    
    return isFullSize() ? 2 : 1;
}

bool HalfSlabTile::isCubeShaped()
{
   
    return isFullSize() != 0;
}

bool HalfSlabTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face) 
{
	if (isFullSize()) return Tile::shouldRenderFace(level, x, y, z, face);

	if (face != Facing::UP && face != Facing::DOWN && !Tile::shouldRenderFace(level, x, y, z, face)) 
	{
		return false;
	}

	int ox = x, oy = y, oz = z;
	ox += Facing::STEP_X[Facing::OPPOSITE_FACING[face]];
	oy += Facing::STEP_Y[Facing::OPPOSITE_FACING[face]];
	oz += Facing::STEP_Z[Facing::OPPOSITE_FACING[face]];

	boolean isUpper = (level->getData(ox, oy, oz) & TOP_SLOT_BIT) != 0;
	if (isUpper) 
	{
		if (face == Facing::DOWN) return true;
		if (face == Facing::UP && Tile::shouldRenderFace(level, x, y, z, face)) return true;
		return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) != 0);
	} 
	else 
	{
		if (face == Facing::UP) return true;
		if (face == Facing::DOWN && Tile::shouldRenderFace(level, x, y, z, face)) return true;
		return !(isHalfSlab(level->getTile(x, y, z)) && (level->getData(x, y, z) & TOP_SLOT_BIT) == 0);
	}
}

int HalfSlabTile::getSpawnResourcesAuxValue(int data)
{
    return data & TYPE_MASK;
}

int HalfSlabTile::cloneTileData(Level *level, int x, int y, int z)
{
    
    return level->getData(x, y, z) & TYPE_MASK;
}

int HalfSlabTile::cloneTileId(Level *level, int x, int y, int z)
{
    return id;
}

int HalfSlabTile::getPlacedOnFaceDataValue(
    Level *level, int x, int y, int z,
    int face, float clickX, float clickY, float clickZ,
    int itemValue)
{
    
    if (face == Facing::DOWN)
        return itemValue | TOP_SLOT_BIT;
    if (face == Facing::UP)
        return itemValue;
    
    return (clickY > 0.5f) ? (itemValue | TOP_SLOT_BIT) : itemValue;
}

 bool HalfSlabTile::isHalfSlab(int tileId)
{
    if (tileId <= 0 || Tile::tiles[tileId] == nullptr)
        return false;
    return dynamic_cast<HalfSlabTile *>(Tile::tiles[tileId]) != nullptr
        && !Tile::tiles[tileId]->isCubeShaped();
}