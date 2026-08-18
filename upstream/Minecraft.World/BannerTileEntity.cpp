#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "BannerTileEntity.h"
#include "TileEntityDataPacket.h"

BannerTileEntity::BannerTileEntity() : TileEntity()
{
	baseColor = 15;
	m_isWall = false;
}

void BannerTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putInt(L"Base", baseColor);
	tag->putByte(L"IsWall", m_isWall ? 1 : 0);

	if (!patterns.empty())
	{
		ListTag<CompoundTag> *list = new ListTag<CompoundTag>();
		for (const BannerPattern &bp : patterns)
		{
			CompoundTag *entry = new CompoundTag();
			entry->putString(L"Pattern", bp.pattern);
			entry->putInt(L"Color", bp.color);
			list->add(entry);
		}
		tag->put(L"Patterns", list);
	}
}

void BannerTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	baseColor = tag->getInt(L"Base");
	m_isWall = tag->contains(L"IsWall") && tag->getByte(L"IsWall") != 0;
	patterns.clear();

	if (tag->contains(L"Patterns"))
	{
		ListTag<CompoundTag> *list = static_cast<ListTag<CompoundTag> *>(
			static_cast<void *>(tag->getList(L"Patterns")));
		if (list)
		{
			for (int i = 0; i < list->size(); i++)
			{
				CompoundTag *entry = list->get(i);
				BannerPattern bp;
				bp.pattern = entry->getString(L"Pattern");
				bp.color = entry->getInt(L"Color");
				patterns.push_back(bp);
				if (patterns.size() >= 6) break;
			}
		}
	}
}

shared_ptr<Packet> BannerTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_BANNER, tag);
}

shared_ptr<TileEntity> BannerTileEntity::clone()
{
	shared_ptr<BannerTileEntity> result = std::make_shared<BannerTileEntity>();
	TileEntity::clone(result);
	result->baseColor = baseColor;
	result->m_isWall = m_isWall;
	result->patterns = patterns;
	return result;
}
