#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.stats.h"
#include "SlimeTile.h"
#include "Entity.h"

SlimeTile::SlimeTile(int id) : HalfTransparentTile(id, L"slime", Material::clay, false)
{
	friction = 0.8f;
}

int SlimeTile::getRenderLayer()
{
	return 2;
}

int SlimeTile::getRenderShape()
{
	return Tile::SHAPE_SLIME;
}

bool SlimeTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	return true;
}

bool SlimeTile::isSolidRender()
{
    return false;
}

int SlimeTile::getPistonPushReaction()
{
	return Material::PUSH_SLIME;
}

void SlimeTile::fallOn(Level *level, int x, int y, int z,
                       shared_ptr<Entity> entity, float distance)
{
    assert(entity != nullptr);

    bool sneaking = entity->isSneaking();

    if (!sneaking)
    {
        entity->causeFallDamage(distance, 0.0f); // no damage
    }
    else 
    {
        Tile::fallOn(level, x, y, z, entity, distance); // normal fall damage
    }
}

void SlimeTile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
    assert(entity != nullptr);

    if ((std::abs)(entity->yd) < 0.1) {
        bool sneaking = entity->isSneaking();

        if (!sneaking) {
            double factor = (std::abs)(entity->yd) * 0.2 + 0.4;

            entity->xd *= factor;
            entity->zd *= factor;
        }
    }

    Tile::stepOn(level, x, y, z, entity);
}

void SlimeTile::updateEntityAfterFallOn(Level* level, shared_ptr<Entity> entity)
{
    assert(entity != nullptr);

    bool sneaking = entity->isSneaking();

    if (!sneaking) {
        if (!(entity->yd > -0.08 && entity->yd < 0.0))
        {
            if (entity->yd < 0.0) 
            {
                entity->yd = -entity->yd; // bounce
            }
        }
        else
        {
            Tile::updateEntityAfterFallOn(level, entity); // LAB_02a7a4e4 goto
        }
    }
    else
    {
        Tile::updateEntityAfterFallOn(level, entity); // LAB_02a7a4e4 definition
    }
}
