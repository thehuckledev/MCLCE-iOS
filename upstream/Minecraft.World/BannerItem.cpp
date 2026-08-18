#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "ItemInstance.h"
#include "BannerItem.h"
#include "BannerTileEntity.h"
#include "com.mojang.nbt.h"
#include "Mth.h"

const unsigned int BannerItem::COLOR_DESCS[16] =
{
	IDS_TILE_BANNER_WHITE,
	IDS_TILE_BANNER_ORANGE,
	IDS_TILE_BANNER_MAGENTA,
	IDS_TILE_BANNER_LIGHT_BLUE,
	IDS_TILE_BANNER_YELLOW,
	IDS_TILE_BANNER_LIME,
	IDS_TILE_BANNER_PINK,
	IDS_TILE_BANNER_GRAY,
	IDS_TILE_BANNER_SILVER,
	IDS_TILE_BANNER_CYAN,
	IDS_TILE_BANNER_PURPLE,
	IDS_TILE_BANNER_BLUE,
	IDS_TILE_BANNER_BROWN,
	IDS_TILE_BANNER_GREEN,
	IDS_TILE_BANNER_RED,
	IDS_TILE_BANNER_BLACK,
};

BannerItem::BannerItem(int id) : Item(id)
{
	maxStackSize = 16;
	setStackedByData(true);
	setMaxDamage(0);
}

unsigned int BannerItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int color = instance->getAuxValue() & 15;
	return COLOR_DESCS[color];
}

bool BannerItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level,
	int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (face == 0) return false;
	if (!level->getMaterial(x, y, z)->isSolid()) return false;

	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	if (!bTestUseOnOnly)
	{
		if (face == 1)
		{
			int rot = Mth::floor(((player->yRot + 180.0f) * 16.0f) / 360.0f + 0.5f) & 15;
			level->setTileAndData(x, y, z, Tile::standing_banner_Id, rot, Tile::UPDATE_CLIENTS);
		}
		else
		{
			level->setTileAndData(x, y, z, Tile::standing_banner_Id, face, Tile::UPDATE_CLIENTS);
		}

		shared_ptr<BannerTileEntity> bte = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
		if (bte != nullptr)
		{
			if (face != 1) bte->setIsWall(true);
			int color = 15 - (instance->getAuxValue() & 15);
			bte->setBaseColor(color);

			if (instance->hasTag())
			{
				CompoundTag* tag = instance->getTag();
				if (tag->contains(L"BlockEntityTag", Tag::TAG_Compound))
				{
					CompoundTag* bet = tag->getCompound(L"BlockEntityTag");
					if (bet->contains(L"Patterns"))
					{
						ListTag<CompoundTag>* list = static_cast<ListTag<CompoundTag>*>(
							static_cast<void*>(bet->getList(L"Patterns")));
						if (list)
						{
							for (int j = 0; j < list->size() && j < 6; j++)
							{
								CompoundTag* entry = list->get(j);
								bte->addPattern(entry->getString(L"Pattern"), entry->getInt(L"Color"));
							}
						}
					}
				}
			}

			bte->setChanged();
		}

		instance->count--;
	}
	return true;
}
