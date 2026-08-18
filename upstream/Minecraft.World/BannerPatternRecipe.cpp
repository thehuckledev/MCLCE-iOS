#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "BannerItem.h"
#include "BannerPatternRecipe.h"
#include "Rose.h"
#include "com.mojang.nbt.h"
#include "ItemInstance.h"
#include "Recipes.h"


const BannerPatternRecipe::PatternInfo BannerPatternRecipe::PATTERNS[] =
{
    { L"bl",  {"   ", "   ", "#  "}, 0,  -1 },
    { L"br",  {"   ", "   ", "  #"}, 0,  -1 },
    { L"tl",  {"#  ", "   ", "   "}, 0,  -1 },
    { L"tr",  {"  #", "   ", "   "}, 0,  -1 },
    { L"bs",  {"   ", "   ", "###"}, 0,  -1 },
    { L"ts",  {"###", "   ", "   "}, 0,  -1 },
    { L"ls",  {"#  ", "#  ", "#  "}, 0,  -1 },
    { L"rs",  {"  #", "  #", "  #"}, 0,  -1 },
    { L"cs",  {" # ", " # ", " # "}, 0,  -1 },
    { L"ms",  {"   ", "###", "   "}, 0,  -1 },
    { L"drs", {"#  ", " # ", "  #"}, 0,  -1 },
    { L"dls", {"  #", " # ", "#  "}, 0,  -1 },
    { L"ss",  {"# #", "# #", "   "}, 0,  -1 },
    { L"cr",  {"# #", " # ", "# #"}, 0,  -1 },
    { L"sc",  {" # ", "###", " # "}, 0,  -1 },
    { L"bt",  {"   ", " # ", "# #"}, 0,  -1 },
    { L"tt",  {"# #", " # ", "   "}, 0,  -1 },
    { L"bts", {"   ", "# #", " # "}, 0,  -1 },
    { L"tts", {" # ", "# #", "   "}, 0,  -1 },
    { L"ld",  {"## ", "#  ", "   "}, 0,  -1 },
    { L"rd",  {"   ", "  #", " ##"}, 0,  -1 },
    { L"lud", {"   ", "#  ", "## "}, 0,  -1 },
    { L"rud", {" ##", "  #", "   "}, 0,  -1 },
    { L"mc",  {"   ", " # ", "   "}, 0,  -1 },
    { L"mr",  {" # ", "# #", " # "}, 0,  -1 },
    { L"vh",  {"## ", "## ", "## "}, 0,  -1 },
    { L"hh",  {"###", "###", "   "}, 0,  -1 },
    { L"vhr", {" ##", " ##", " ##"}, 0,  -1 },
    { L"hhb", {"   ", "###", "###"}, 0,  -1 },
    { L"bo",  {"###", "# #", "###"}, 0,  -1 },
    { L"gra", {"# #", " # ", " # "}, 0,  -1 },
    { L"gru", {" # ", " # ", "# #"}, 0,  -1 },
    { L"cbo", {nullptr, nullptr, nullptr}, Tile::vine_Id,          -1 },
    { L"bri", {nullptr, nullptr, nullptr}, Tile::brick_block_Id,   -1 },
    { L"cre", {nullptr, nullptr, nullptr}, Item::skull_Id,          4 },
    { L"sku", {nullptr, nullptr, nullptr}, Item::skull_Id,          1 },
    { L"flo", {nullptr, nullptr, nullptr}, Tile::red_flower_Id, Rose::OXEYE_DAISY },
    { L"moj", {nullptr, nullptr, nullptr}, Item::golden_apple_Id,   1 },
};

const int BannerPatternRecipe::PATTERN_COUNT = static_cast<int>(sizeof(PATTERNS) / sizeof(PATTERNS[0]));

int BannerPatternRecipe::getBannerPatternCount(shared_ptr<ItemInstance> banner)
{
    if (!banner->hasTag()) return 0;
    CompoundTag* tag = banner->getTag();
    if (!tag->contains(L"BlockEntityTag", Tag::TAG_Compound)) return 0;
    CompoundTag* bet = tag->getCompound(L"BlockEntityTag");
    if (!bet->contains(L"Patterns")) return 0;
    ListTag<Tag>* list = bet->getList(L"Patterns");
    return list ? list->size() : 0;
}

const wchar_t* BannerPatternRecipe::matchPattern(shared_ptr<CraftingContainer> craftSlots, int& outDyeColor)
{
    int slots = craftSlots->getContainerSize();

    for (int p = 0; p < PATTERN_COUNT; p++)
    {
        const PatternInfo& pat = PATTERNS[p];
        bool gridBased = (pat.rows[0] != nullptr);

        if (gridBased)
        {
            if (slots != 9) continue;

            int dyeColor = -1;
            bool ok = true;

            for (int k = 0; k < 9 && ok; k++)
            {
                int row = k / 3;
                int col = k % 3;
                shared_ptr<ItemInstance> item = craftSlots->getItem(k);
                char cell = pat.rows[row][col];

                if (item != nullptr && dynamic_cast<BannerItem*>(item->getItem()) == nullptr)
                {
                    if (item->id != Item::dye_Id) { ok = false; break; }
                    if (cell == ' ')              { ok = false; break; }
                    if (dyeColor != -1 && dyeColor != item->getAuxValue()) { ok = false; break; }
                    dyeColor = item->getAuxValue();
                }
                else
                {
                    if (cell != ' ') { ok = false; break; }
                }
            }

            if (ok && dyeColor != -1)
            {
                outDyeColor = dyeColor;
                return pat.id;
            }
        }
        else
        {
            // item-based pattern: needs banner + dye + one special item, classic crafting only
            if (slots != 9) continue;

            bool hasSpecial = false;
            int  dyeColor   = -1;
            bool ok         = true;

            for (int k = 0; k < slots && ok; k++)
            {
                shared_ptr<ItemInstance> item = craftSlots->getItem(k);
                if (item == nullptr) continue;

                if (dynamic_cast<BannerItem*>(item->getItem()) != nullptr) continue;

                if (item->id == Item::dye_Id)
                {
                    if (dyeColor != -1) { ok = false; break; }
                    dyeColor = item->getAuxValue();
                }
                else if (item->id == pat.specialItemId &&
                         (pat.specialItemAux == -1 || item->getAuxValue() == pat.specialItemAux))
                {
                    if (hasSpecial) { ok = false; break; }
                    hasSpecial = true;
                }
                else
                {
                    ok = false; break;
                }
            }

            if (ok && hasSpecial && dyeColor != -1)
            {
                outDyeColor = dyeColor;
                return pat.id;
            }
        }
    }

    return nullptr;
}

bool BannerPatternRecipe::matches(shared_ptr<CraftingContainer> craftSlots, Level* level)
{
    if (craftSlots->getContainerSize() != 9) return false;

    shared_ptr<ItemInstance> banner = nullptr;

    for (int i = 0; i < craftSlots->getContainerSize(); i++)
    {
        shared_ptr<ItemInstance> item = craftSlots->getItem(i);
        if (item == nullptr) continue;

        if (dynamic_cast<BannerItem*>(item->getItem()) != nullptr)
        {
            if (banner != nullptr) return false;
            if (getBannerPatternCount(item) >= 6) return false;
            banner = item;
        }
    }

    if (banner == nullptr) return false;

    int dyeColor = -1;
    return matchPattern(craftSlots, dyeColor) != nullptr;
}

shared_ptr<ItemInstance> BannerPatternRecipe::assemble(shared_ptr<CraftingContainer> craftSlots)
{
    if (craftSlots == nullptr)
        return std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 0);

    shared_ptr<ItemInstance> banner = nullptr;

    for (int i = 0; i < craftSlots->getContainerSize(); i++)
    {
        shared_ptr<ItemInstance> item = craftSlots->getItem(i);
        if (item != nullptr && dynamic_cast<BannerItem*>(item->getItem()) != nullptr)
        {
            banner = item;
            break;
        }
    }

    if (banner == nullptr) return nullptr;

    int dyeColor = -1;
    const wchar_t* patternId = matchPattern(craftSlots, dyeColor);
    if (patternId == nullptr || dyeColor == -1) return nullptr;

    shared_ptr<ItemInstance> result = banner->copy();
    result->count = 1;

    if (!result->hasTag()) result->setTag(new CompoundTag());
    CompoundTag* tag = result->getTag();

    if (!tag->contains(L"BlockEntityTag", Tag::TAG_Compound))
        tag->putCompound(L"BlockEntityTag", new CompoundTag());
    CompoundTag* bet = tag->getCompound(L"BlockEntityTag");

    ListTag<CompoundTag>* list;
    if (bet->contains(L"Patterns"))
    {
        list = static_cast<ListTag<CompoundTag>*>(static_cast<void*>(bet->getList(L"Patterns")));
    }
    else
    {
        list = new ListTag<CompoundTag>();
        bet->put(L"Patterns", list);
    }

    CompoundTag* entry = new CompoundTag();
    entry->putString(L"Pattern", patternId);
    entry->putInt(L"Color", dyeColor);
    list->add(entry);

    return result;
}

int BannerPatternRecipe::size()
{
    return 10;
}

const ItemInstance* BannerPatternRecipe::getResultItem()
{
    return nullptr;
}

const int BannerPatternRecipe::getGroup()
{
    return eGroupType_Max;
}

bool BannerPatternRecipe::reqs(int iRecipe)
{
    return false;
}

void BannerPatternRecipe::reqs(INGREDIENTS_REQUIRED* pIngReq)
{
    pIngReq->iIngC       = 0;
    pIngReq->iType       = RECIPE_TYPE_3x3;
    pIngReq->iIngIDA     = new int[1]();
    pIngReq->iIngValA    = new int[1]();
    pIngReq->iIngAuxValA = new int[1]();
    pIngReq->uiGridA     = new unsigned int[9]();

    pIngReq->pRecipy = this;
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
        pIngReq->bCanMake[i] = false;
}

void BannerPatternRecipe::writeToStream(DataOutputStream* dos)
{
    dos->writeByte(99);
}
