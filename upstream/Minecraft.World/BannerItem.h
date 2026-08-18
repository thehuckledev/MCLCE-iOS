#pragma once
#include "Item.h"

class IconRegister;

class BannerItem : public Item
{
private:
	static const unsigned int COLOR_DESCS[16];

public:
	BannerItem(int id);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance) override;
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly = false) override;
};
