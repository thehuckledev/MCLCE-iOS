#include "stdafx.h"
#include "BannerRenderer.h"
#include "BannerModel.h"
#include "ModelPart.h"
#include "TileEntityRenderDispatcher.h"
#include "Textures.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/DyePowderItem.h"
#include "../Minecraft.World/Entity.h"
#include "../Minecraft.World/Level.h"
#include "../Minecraft.World/ItemInstance.h"
#include "../Minecraft.World/com.mojang.nbt.h"
#include "../Minecraft.World/BannerTileEntity.h"
#include "Lighting.h"
#include <cmath>
#include <string>

BannerRenderer *BannerRenderer::instance = nullptr;
ResourceLocation BannerRenderer::BANNER_LOCATION = ResourceLocation(TN_MOB_BANNER_BANNER_BASE);

BannerRenderer::BannerRenderer()
{
	bannerModel = new BannerModel();
}

void BannerRenderer::init(TileEntityRenderDispatcher *dispatcher)
{
	TileEntityRenderer::init(dispatcher);
	instance = this;
}

static void applyBannerColor(int baseColor, float alpha, float brightness = 1.0f)
{
	if (baseColor >= 0 && baseColor < 16)
	{
		int rgb = DyePowderItem::COLOR_RGB[baseColor];
		float r = ((rgb >> 16) & 0xFF) / 255.0f * brightness;
		float g = ((rgb >>  8) & 0xFF) / 255.0f * brightness;
		float b = ( rgb        & 0xFF) / 255.0f * brightness;
		glColor4f(r, g, b, alpha);
	}
	else
	{
		glColor4f(brightness, brightness, brightness, alpha);
	}
}

const wchar_t *BannerRenderer::patternTextureName(const std::wstring &id)
{
	static const struct { const wchar_t *id; const wchar_t *tex; } MAP[] =
	{
		{ L"bl",  L"square_bottom_left"  }, { L"br",  L"square_bottom_right" },
		{ L"tl",  L"square_top_left"     }, { L"tr",  L"square_top_right"    },
		{ L"bs",  L"stripe_bottom"       }, { L"ts",  L"stripe_top"          },
		{ L"ls",  L"stripe_left"         }, { L"rs",  L"stripe_right"        },
		{ L"cs",  L"stripe_center"       }, { L"ms",  L"stripe_middle"       },
		{ L"drs", L"stripe_downright"    }, { L"dls", L"stripe_downleft"     },
		{ L"ss",  L"small_stripes"       }, { L"cr",  L"cross"               },
		{ L"sc",  L"straight_cross"      }, { L"bt",  L"triangle_bottom"     },
		{ L"tt",  L"triangle_top"        }, { L"bts", L"triangles_bottom"    },
		{ L"tts", L"triangles_top"       }, { L"ld",  L"diagonal_left"       },
		{ L"rd",  L"diagonal_up_right"   }, { L"lud", L"diagonal_up_left"    },
		{ L"rud", L"diagonal_right"      }, { L"mc",  L"circle"              },
		{ L"mr",  L"rhombus"             }, { L"vh",  L"half_vertical"       },
		{ L"hh",  L"half_horizontal"     }, { L"vhr", L"half_vertical_right" },
		{ L"hhb", L"half_horizontal_bottom" }, { L"bo", L"border"           },
		{ L"cbo", L"curly_border"        }, { L"cre", L"creeper"             },
		{ L"gra", L"gradient"            }, { L"gru", L"gradient_up"         },
		{ L"bri", L"bricks"             }, { L"sku", L"skull"               },
		{ L"flo", L"flower"              }, { L"moj", L"mojang"              },
	};
	for (const auto &e : MAP)
	{
		if (id == e.id) return e.tex;
	}
	return nullptr;
}

std::vector<BannerPattern> BannerRenderer::patternsFromItem(shared_ptr<ItemInstance> bannerItem)
{
	std::vector<BannerPattern> out;
	if (bannerItem == nullptr || !bannerItem->hasTag()) return out;

	CompoundTag *tag = bannerItem->getTag();
	if (!tag->contains(L"BlockEntityTag", Tag::TAG_Compound)) return out;
	CompoundTag *bet = tag->getCompound(L"BlockEntityTag");
	if (!bet->contains(L"Patterns")) return out;

	ListTag<CompoundTag> *list = static_cast<ListTag<CompoundTag> *>(
		static_cast<void *>(bet->getList(L"Patterns")));
	if (!list) return out;

	for (int i = 0; i < list->size() && i < 6; i++)
	{
		CompoundTag *entry = list->get(i);
		BannerPattern bp;
		bp.pattern = entry->getString(L"Pattern");
		bp.color = entry->getInt(L"Color");
		out.push_back(bp);
	}
	return out;
}

void BannerRenderer::renderPatterns(const std::vector<BannerPattern> &patterns, float alpha, float lightU, float lightV)
{
	if (patterns.empty()) return;

	Textures *textures = tileEntityRenderDispatcher ? tileEntityRenderDispatcher->textures : nullptr;
	if (textures == nullptr) return;

	float restoreU, restoreV;
	glGetLightmapUV(restoreU, restoreV);
	if (lightU >= 0.0f) { restoreU = lightU; restoreV = lightV; }

	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	for (const BannerPattern &bp : patterns)
	{
		const wchar_t *tex = patternTextureName(bp.pattern);
		if (tex == nullptr) continue;

		ResourceLocation loc(std::wstring(L"mob/banner/") + tex + L".png");
		textures->bindTexture(&loc);

		glMultiTexCoord2f(GL_TEXTURE1, 240.0f, 240.0f);
		glColorMask(true, true, true, false);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		bannerModel->bannerSlate->render(1.0f / 16.0f, false);
		glColorMask(true, true, true, true);

		glMultiTexCoord2f(GL_TEXTURE1, restoreU, restoreV);
		glBlendFunc(GL_ONE, GL_ONE);
		applyBannerColor(bp.color, alpha);
		bannerModel->bannerSlate->render(1.0f / 16.0f, false);
	}

	glDisable(GL_BLEND);
}

void BannerRenderer::renderModelColoured(int baseColor, const std::vector<BannerPattern> &patterns, float alpha, bool useCompiled, float lightU, float lightV)
{
	bindTexture(&BANNER_LOCATION);

	glDisable(GL_BLEND);
	applyBannerColor(baseColor, alpha);
	bannerModel->bannerSlate->render(1.0f / 16.0f, false);

	renderPatterns(patterns, alpha, lightU, lightV);

	bindTexture(&BANNER_LOCATION);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	bannerModel->bannerStand->render(1.0f / 16.0f, useCompiled);
	bannerModel->bannerTop->render(1.0f / 16.0f, useCompiled);
}

void BannerRenderer::render(shared_ptr<TileEntity> _banner, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(_banner);
	if (!banner) return;

	Tile *tile = banner->getTile();
	if (tile != Tile::standing_banner) return;
	const float f = 0.6666667f;

	glDisable(GL_LIGHTING);
	glPushMatrix();

	if (!banner->isWall())
	{
		glTranslatef((float)x + 0.5f, (float)y + 0.75f * f, (float)z + 0.5f);
		float rot = banner->getData() * 360.0f / 16.0f;
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		bannerModel->bannerStand->visible = true;
	}
	else
	{
		glTranslatef((float)x + 0.5f, (float)y - 0.25f * f, (float)z + 0.5f);
		int face = banner->getData();
		float rot = 0.0f;
		if (face == 2) rot = 180.0f;
		if (face == 4) rot =  90.0f;
		if (face == 5) rot = -90.0f;
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -0.3125f, -0.4375f);
		bannerModel->bannerStand->visible = false;
	}

	bannerModel->bannerSlate->y = -32.0f;

	float f3 = (float)(banner->x * 7 + banner->y * 9 + banner->z * 13)
	         + (float)banner->level->getGameTime() + a;
	bannerModel->bannerSlate->xRot = (-0.0125f + 0.01f * cosf(f3 * 3.14159265f * 0.02f)) * 3.14159265f;

	glScalef(f, -f, -f);
	int lightCol = banner->level->getLightColor(banner->x, banner->y, banner->z, 0);
	float lightU = (float)(lightCol & 0xFFFF);
	float lightV = (float)((lightCol >> 16) & 0xFFFF);
	renderModelColoured(banner->getBaseColor(), banner->getPatterns(), alpha, useCompiled, lightU, lightV);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glMultiTexCoord2f(GL_TEXTURE1, 240.0f, 240.0f);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void BannerRenderer::renderBannerForGui(float x, float y, float scaleX, float scaleY, float alpha, int baseColor, shared_ptr<ItemInstance> bannerItem)
{
	std::vector<BannerPattern> patterns = patternsFromItem(bannerItem);

	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL);

	glTranslatef(x, y, 0.0f);
	glScalef(16.0f * scaleX, 16.0f * scaleY, 1.0f);

	glTranslatef(0.5f, 0.72f, 0.0f);

	glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(199.0f, 0.0f, 1.0f, 0.0f);
	glScalef(-1.0f, 1.0f, 1.0f);

	const float s = 0.33f;
	glScalef(s, s, s);

	bannerModel->bannerSlate->y   = -32.0f;
	bannerModel->bannerSlate->xRot = 0.0f;
	bannerModel->bannerStand->visible = true;

	Lighting::setAmbient(0.6f);
	renderModelColoured(baseColor, patterns, alpha, false);
	Lighting::setAmbient(0.4f);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
}

void BannerRenderer::renderBannerForHand(float alpha, int baseColor, shared_ptr<ItemInstance> bannerItem)
{
	std::vector<BannerPattern> patterns = patternsFromItem(bannerItem);

	glDisable(GL_LIGHTING);
	glPushMatrix();

	bannerModel->bannerSlate->y   = -32.0f;
	bannerModel->bannerSlate->xRot = 0.0f;
	bannerModel->bannerStand->visible = true;

	renderModelColoured(baseColor, patterns, alpha, false);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}
