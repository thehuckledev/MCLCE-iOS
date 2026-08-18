#pragma once
#include "TileEntityRenderer.h"
#include "../Minecraft.World/BannerTileEntity.h"
#include <vector>
class BannerModel;
class ItemInstance;

class BannerRenderer : public TileEntityRenderer
{
public:
	static BannerRenderer *instance;

private:
	static ResourceLocation BANNER_LOCATION;
	BannerModel *bannerModel;

	void renderModelColoured(int baseColor, const std::vector<BannerPattern> &patterns, float alpha, bool useCompiled, float lightU = -1.0f, float lightV = -1.0f);
	void renderPatterns(const std::vector<BannerPattern> &patterns, float alpha, float lightU = -1.0f, float lightV = -1.0f);

	static std::vector<BannerPattern> patternsFromItem(shared_ptr<ItemInstance> bannerItem);
	static const wchar_t *patternTextureName(const std::wstring &patternId);

public:
	BannerRenderer();
	virtual void init(TileEntityRenderDispatcher *dispatcher) override;
	virtual void render(shared_ptr<TileEntity> banner, double x, double y, double z, float a, bool setColor, float alpha = 1.0f, bool useCompiled = true);

	void renderBannerForGui(float x, float y, float scaleX, float scaleY, float alpha, int baseColor, shared_ptr<ItemInstance> bannerItem = nullptr);
	void renderBannerForHand(float alpha, int baseColor, shared_ptr<ItemInstance> bannerItem = nullptr);
};
