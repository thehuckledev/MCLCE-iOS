#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

struct LootTableConditionDefinition
{
    std::string conditionName;
    std::string entity;
    std::string propertyName;
    std::string propertyValue;
};

struct LootTableFunctionDefinition
{
    std::string functionName;
    int minCount = 0;
    int maxCount = 0;
    std::vector<LootTableConditionDefinition> conditions;

    std::string nbtTag;       // set_nbt
    int limit = 0;            // looting_enchant
    bool treasureOnly = false; // enchant_with_levels

    double minDamageFraction = 0.0; // set_damage
	double maxDamageFraction = 0.0; // set_damage
};

struct LootTableEntryDefinition
{
    std::string type;
    std::string name;
    int weight = 1;
    int quality = 0; // only used by type="loot_table" entries it seems like
    std::vector<LootTableFunctionDefinition> functions;
};

struct LootTablePoolConditionDefinition
{
    std::string conditionName;
    double chance = 0.0;
    double lootingMultiplier = 0.0;
};

struct LootTablePoolDefinition
{
    int rolls = 1;
    int minRolls = 1;
    int maxRolls = 1;
    std::vector<LootTableEntryDefinition> entries;
    std::vector<LootTablePoolConditionDefinition> conditions;
};

struct LootTableDefinition
{
    std::string path;
    std::vector<LootTablePoolDefinition> pools;
};

struct LootTableDropResult
{
    int itemId = 0;
    int count = 0;

    int data = 0;                  // set_data
	int damage = 0;                // unused (replaced by set_damage)
	double damageFraction = 0.0;   // set_damage: 0.0 = undamaged, 1.0 = fully damaged
	std::string nbtTag;            // set_nbt
	int enchantLevels = 0;         // enchant_with_levels: target enchantment level
	bool treasureEnchant = false;  // enchant_with_levels: treasure-only allowlist
};

class LootTableManager
{
public:
    static LootTableManager &Get();

    bool LoadFromDisk(const std::string &baseDirectory = "");
    bool HasLoadedTables() const;

    const LootTableDefinition *GetTable(const std::string &tableName) const;
    std::vector<LootTableDropResult> ResolveDrops(
        const std::string &tableName,
        bool wasKilledByPlayer,
        int playerBonusLevel,
        const std::function<int(int)> &randomInt,
        bool entityIsOnFire = false) const;

private:
    std::vector<LootTableDefinition> m_tables;
    std::unordered_map<std::string, size_t> m_tableIndexByPath;
    std::string m_rootDirectory;
    bool m_loaded = false;
};
