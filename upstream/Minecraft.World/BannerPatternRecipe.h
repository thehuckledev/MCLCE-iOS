#pragma once
#include "Recipy.h"

class BannerPatternRecipe : public Recipy
{
public:
    bool matches(shared_ptr<CraftingContainer> craftSlots, Level* level) override;
    shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots) override;
    int size() override;
    const ItemInstance* getResultItem() override;
    const int getGroup() override;
    bool reqs(int iRecipe) override;
    void reqs(INGREDIENTS_REQUIRED* pIngReq) override;
    void writeToStream(DataOutputStream* dos) override;

private:
    struct PatternInfo
    {
        const wchar_t* id;
        const char*    rows[3];
        int            specialItemId;
        int            specialItemAux;
    };

    static const PatternInfo PATTERNS[];
    static const int         PATTERN_COUNT;

    static int          getBannerPatternCount(shared_ptr<ItemInstance> banner);
    const wchar_t*      matchPattern(shared_ptr<CraftingContainer> craftSlots, int& outDyeColor);
};
