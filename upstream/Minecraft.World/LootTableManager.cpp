#include "stdafx.h"
#include "LootTableManager.h"
#include "StringHelpers.h"
#include "net.minecraft.world.item.h"
#include "../generated/ItemNameMap.h"

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Xbox/XML/ATGXmlParser.h"

namespace fs = std::filesystem;

// set to one to enable the yapper
#ifndef LOOT_TABLE_VERBOSE_LOGGING
#define LOOT_TABLE_VERBOSE_LOGGING 0
#endif

static bool IsValidDirectory(const fs::path &path)
{
    std::error_code ec;
    return fs::exists(path, ec) && !ec && fs::is_directory(path, ec) && !ec;
}

namespace
{
// route everything through printf cause debugprintf sucks on linux
void LootLog(const char *format, ...)
{
#if LOOT_TABLE_VERBOSE_LOGGING
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    printf("%s", buffer);
#else
    (void)format;
#endif
}

std::string ToUtf8(const WCHAR *value, UINT length)
{
    if (value == nullptr || length == 0)
    {
        return std::string();
    }

    std::string result;
    result.reserve(length);
    for (UINT i = 0; i < length; ++i)
    {
        result.push_back(static_cast<char>(value[i] & 0xFF));
    }
    return result;
}

bool IsXmlFileName(const std::wstring &path)
{
    if (path.length() < 4)
    {
        return false;
    }

    return toLower(path.substr(path.length() - 4)) == L".xml";
}

std::string TrimString(const std::string &value)
{
    size_t start = 0;
    size_t end = value.size();
    while (start < end && std::isspace(static_cast<unsigned char>(value[start])))
    {
        start++;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
    {
        end--;
    }
    return value.substr(start, end - start);
}

int ParseInt(const std::string &value)
{
    std::stringstream stream(value);
    int parsed = 0;
    stream >> parsed;
    return parsed;
}

double ParseFloat(const std::string &value)
{
    std::stringstream stream(value);
    double parsed = 0.0;
    stream >> parsed;
    return parsed;
}

bool RandomChance(double chance, const std::function<int(int)> &randomInt)
{
    if (chance <= 0.0)
    {
        return false;
    }
    if (chance >= 1.0)
    {
        return true;
    }

    const int scale = 1000000;
    const int threshold = static_cast<int>(chance * scale + 0.5);
    return randomInt(scale) < threshold;
}

std::string NormalizeTableName(const std::string &tableName)
{
    std::string normalized = tableName;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    if (!normalized.empty() && normalized[0] == '/')
    {
        normalized.erase(normalized.begin());
    }

    while (!normalized.empty() && normalized[0] == '.')
    {
        normalized.erase(normalized.begin());
    }

    return normalized;
}

bool IsTrueValue(const std::string &value)
{
    std::string normalized = TrimString(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return normalized == "true" || normalized == "1" || normalized == "yes";
}

bool EvaluateCondition(const LootTableConditionDefinition &condition, bool entityIsOnFire)
{
    if (condition.conditionName == "entity_properties" && condition.entity == "this" && condition.propertyName == "on_fire")
    {
        if (condition.propertyValue.empty())
        {
            return false;
        }
        return entityIsOnFire == IsTrueValue(condition.propertyValue);
    }

    return false;
}

// very hacky
int ResolveSmeltedItemId(const std::string &itemName, int currentItemId)
{
    if (currentItemId == 0)
    {
        return 0;
    }

    const std::string normalized = (itemName.find("minecraft:") == 0) ? itemName.substr(10) : itemName;

    if (normalized == "potato")
    {
        return GetItemIdByName("baked_potato");
    }
    else if (normalized == "beef"
    || normalized == "porkchop"
    || normalized == "chicken"
    || normalized == "mutton"
    || normalized == "rabbit")
    {
        return GetItemIdByName("cooked_" + normalized);
    }

    return currentItemId;
}

void CollectXmlFiles(const std::string &directory, std::vector<std::string> &files)
{
    std::error_code ec;
    for (const fs::directory_entry &entry : fs::recursive_directory_iterator(directory, ec))
    {
        if (ec)
        {
            app.DebugPrintf("LootTableManager::CollectXmlFiles directory iteration failed: %s\n", ec.message().c_str());
            return;
        }

        if (!entry.is_regular_file(ec) || ec)
        {
            continue;
        }

        const fs::path &path = entry.path();
        if (IsXmlFileName(path.wstring()))
        {
            files.push_back(path.string());
        }
    }
}

class XmlLootTableCallback : public ATG::ISAXCallback
{
public:
    explicit XmlLootTableCallback(std::vector<LootTableDefinition> *tables)
        : m_tables(tables)
    {
    }

    HRESULT StartDocument() override
    {
        return S_OK;
    }

    HRESULT EndDocument() override
    {
        return S_OK;
    }

    HRESULT ElementBegin(CONST WCHAR *strName, UINT NameLen, CONST ATG::XMLAttribute *pAttributes, UINT NumAttributes) override
    {
        const std::string elementName = ToUtf8(strName, NameLen);
        m_elementStack.push_back(elementName);

        if (elementName == "loot_table")
        {
            m_tables->push_back(LootTableDefinition());
            m_currentTableIndex = static_cast<int>(m_tables->size() - 1);
            m_currentPoolIndex = -1;
            m_currentEntryIndex = -1;
            m_currentFunctionIndex = -1;
            return S_OK;
        }

        if (m_currentTableIndex < 0)
        {
            return S_OK;
        }

        LootTableDefinition *table = &(*m_tables)[m_currentTableIndex];
        if (elementName == "pool")
        {
            table->pools.push_back(LootTablePoolDefinition());
            m_currentPoolIndex = static_cast<int>(table->pools.size() - 1);
            m_currentEntryIndex = -1;
            m_currentFunctionIndex = -1;
            return S_OK;
        }

        if (m_currentPoolIndex < 0)
        {
            return S_OK;
        }

        LootTablePoolDefinition *pool = &table->pools[m_currentPoolIndex];

        // helper functions in an attempt to kill off repetitiveness
        auto parentIs = [&](const std::string &parent) -> bool {
            return m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == parent;
        };
        auto beginCapture = [&](const std::string &tag) {
            if (m_captureText)
            {
                m_captureStack.emplace_back(m_captureText, m_captureTag);
            }
            m_captureText = true;
            m_captureTag = tag;
            m_captureBuffer.clear();
        };

        // rolls and conditions first
        if (elementName == "rolls")
        {
            m_currentRollCountContext = 0;
            m_currentRollRange = false;
            beginCapture(elementName);
            return S_OK;
        }

        if (elementName == "min" && m_currentFunctionIndex < 0 && m_currentPoolIndex >= 0 && parentIs("rolls"))
        {
            m_currentRollCountContext = 1;
            m_currentRollRange = true;
            beginCapture(elementName);
            return S_OK;
        }

        if (elementName == "max" && m_currentFunctionIndex < 0 && m_currentPoolIndex >= 0 && parentIs("rolls"))
        {
            m_currentRollCountContext = 2;
            m_currentRollRange = true;
            beginCapture(elementName);
            return S_OK;
        }

        if (elementName == "condition" && m_currentFunctionIndex < 0 && parentIs("conditions"))
        {
            pool->conditions.push_back(LootTablePoolConditionDefinition());
            m_currentPoolConditionIndex = static_cast<int>(pool->conditions.size() - 1);
            return S_OK;
        }

        if (m_currentPoolConditionIndex >= 0 && m_currentFunctionIndex < 0 && parentIs("condition"))
        {
            if (elementName == "condition" || elementName == "chance" || elementName == "looting_multiplier")
            {
                beginCapture(elementName);
                return S_OK;
            }
        }

        // entrie as fallback now since the xmls have been 'cleansed'
        if ((elementName == "entry" || elementName == "entrie") && m_currentPoolIndex >= 0)
        {
            pool->entries.push_back(LootTableEntryDefinition());
            m_currentEntryIndex = static_cast<int>(pool->entries.size() - 1);
            m_currentFunctionIndex = -1;
            return S_OK;
        }

        if (m_currentEntryIndex < 0)
        {
            return S_OK;
        }

        LootTableEntryDefinition *entry = &pool->entries[m_currentEntryIndex];
        if (elementName == "function" && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "functions")
        {
            entry->functions.push_back(LootTableFunctionDefinition());
            m_currentFunctionIndex = static_cast<int>(entry->functions.size() - 1);
            m_currentFunctionConditionIndex = -1;
            return S_OK;
        }

        if (elementName == "conditions" && m_currentFunctionIndex >= 0)
        {
            return S_OK;
        }

        if (elementName == "condition" && m_currentFunctionIndex >= 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "conditions")
        {
            LootTableFunctionDefinition *function = &entry->functions[m_currentFunctionIndex];
            function->conditions.push_back(LootTableConditionDefinition());
            m_currentFunctionConditionIndex = static_cast<int>(function->conditions.size() - 1);
            return S_OK;
        }

        if (elementName == "condition" && m_currentFunctionConditionIndex >= 0 && m_currentFunctionIndex >= 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "condition")
        {
            m_captureText = true;
            m_captureTag = elementName;
            return S_OK;
        }

        if (elementName == "entity" && m_currentFunctionConditionIndex >= 0 && m_currentFunctionIndex >= 0)
        {
            m_captureText = true;
            m_captureTag = elementName;
            return S_OK;
        }

        if (elementName == "properties" && m_currentFunctionConditionIndex >= 0 && m_currentFunctionIndex >= 0)
        {
            return S_OK;
        }

        if (m_currentFunctionConditionIndex >= 0 && m_currentFunctionIndex >= 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "properties")
        {
            m_captureText = true;
            m_captureTag = elementName;
            return S_OK;
        }

        // then come entry tags
        if (elementName == "weight" || elementName == "type" || elementName == "name" || elementName == "quality")
        {
            m_captureText = true;
            m_captureTag = elementName;
            return S_OK;
        }

        if (elementName == "function" && m_currentFunctionIndex >= 0)
        {
            m_captureText = true;
            m_captureTag = elementName;
            return S_OK;
        }

        if (elementName == "count")
        {
            m_captureText = false;
            m_captureTag.clear();
            m_currentCountContext = 0;
            return S_OK;
        }

        // <data> (set_data), <damage> (set_damage), and <levels>
        // (enchant_with_levels) can either be plain values and/or min/max ranges
        if ((elementName == "data" || elementName == "damage" || elementName == "levels") && m_currentFunctionIndex >= 0)
        {
            m_captureText = true;
            m_captureTag = elementName;
            m_captureBuffer.clear();
            if (elementName == "damage")
            {
                m_currentDamageContext = true;
            }
            return S_OK;
        }

        // <treasure> (enchant_with_levels), <tag> (set_nbt), and <limit>
        // (looting_enchant) is always a regular value(?)
        if ((elementName == "treasure" || elementName == "tag" || elementName == "limit") && m_currentFunctionIndex >= 0)
        {
            m_captureText = true;
            m_captureTag = elementName;
            m_captureBuffer.clear();
            return S_OK;
        }

        if (elementName == "min")
        {
            if (m_currentFunctionIndex >= 0)
            {
                m_captureText = true;
                m_captureTag = elementName;
                m_currentCountContext = 1;
            }
            return S_OK;
        }

        if (elementName == "max")
        {
            if (m_currentFunctionIndex >= 0)
            {
                m_captureText = true;
                m_captureTag = elementName;
                m_currentCountContext = 2;
            }
            return S_OK;
        }

        return S_OK;
    }

    HRESULT ElementContent(CONST WCHAR *strData, UINT DataLen, BOOL) override
    {
        if (m_captureText && strData != nullptr && DataLen > 0)
        {
            const std::string chunk = ToUtf8(strData, DataLen);
            m_captureBuffer.append(chunk);
        }
        return S_OK;
    }

    HRESULT ElementEnd(CONST WCHAR *strName, UINT NameLen) override
    {
        const std::string elementName = ToUtf8(strName, NameLen);

        if (m_captureText && m_captureTag == elementName)
        {
            const std::string trimmedValue = TrimString(m_captureBuffer);

            if (elementName == "rolls" && m_currentPoolIndex >= 0 && m_currentTableIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].rolls = ParseInt(trimmedValue);
            }
            else if (elementName == "weight" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].weight = ParseInt(trimmedValue);
            }
            else if (elementName == "type" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].type = trimmedValue;
            }
            else if (elementName == "name" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].name = trimmedValue;
            }
            else if (elementName == "condition" && m_currentPoolIndex >= 0 && m_currentPoolConditionIndex >= 0 && m_currentFunctionIndex < 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "condition")
            {
                LootTablePoolConditionDefinition &condition = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].conditions[m_currentPoolConditionIndex];
                condition.conditionName = trimmedValue;
            }
            else if (elementName == "chance" && m_currentPoolIndex >= 0 && m_currentPoolConditionIndex >= 0 && m_currentFunctionIndex < 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "condition")
            {
                LootTablePoolConditionDefinition &condition = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].conditions[m_currentPoolConditionIndex];
                condition.chance = ParseFloat(trimmedValue);
            }
            else if (elementName == "looting_multiplier" && m_currentPoolIndex >= 0 && m_currentPoolConditionIndex >= 0 && m_currentFunctionIndex < 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "condition")
            {
                LootTablePoolConditionDefinition &condition = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].conditions[m_currentPoolConditionIndex];
                condition.lootingMultiplier = ParseFloat(trimmedValue);
            }
            else if (elementName == "function" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                // just in case
                std::string functionName = trimmedValue;
                if (functionName.find("minecraft:") == 0)
                {
                    functionName = functionName.substr(10);
                }
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex].functionName = functionName;
            }
            else if (elementName == "condition" && m_currentFunctionConditionIndex >= 0 && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                function.conditions[m_currentFunctionConditionIndex].conditionName = trimmedValue;
            }
            else if (elementName == "entity" && m_currentFunctionConditionIndex >= 0 && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                function.conditions[m_currentFunctionConditionIndex].entity = trimmedValue;
            }
            else if (m_currentFunctionConditionIndex >= 0 && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "properties")
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                function.conditions[m_currentFunctionConditionIndex].propertyName = elementName;
                function.conditions[m_currentFunctionConditionIndex].propertyValue = trimmedValue;
            }
            else if (elementName == "min" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                if (m_currentDamageContext)
                {
                    function.minDamageFraction = ParseFloat(trimmedValue);
                }
                else
                {
                    function.minCount = ParseInt(trimmedValue);
                }
            }
            else if (elementName == "max" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                if (m_currentDamageContext)
                {
                    function.maxDamageFraction = ParseFloat(trimmedValue);
                }
                else
                {
                    function.maxCount = ParseInt(trimmedValue);
                }
            }
            else if ((elementName == "data" || elementName == "damage" || elementName == "levels") && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                LootTableFunctionDefinition &function = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex];
                if (elementName == "damage")
                {
                    const double value = ParseFloat(trimmedValue);
                    function.minDamageFraction = value;
                    function.maxDamageFraction = value;
                }
                else
                {
                    const int value = ParseInt(trimmedValue);
                    function.minCount = value;
                    function.maxCount = value;
                }
            }
            else if (elementName == "treasure" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex].treasureOnly = IsTrueValue(trimmedValue);
            }
            else if (elementName == "tag" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex].nbtTag = trimmedValue;
            }
            else if (elementName == "limit" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0 && m_currentFunctionIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].functions[m_currentFunctionIndex].limit = ParseInt(trimmedValue);
            }
            else if (elementName == "quality" && m_currentPoolIndex >= 0 && m_currentEntryIndex >= 0)
            {
                (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex].entries[m_currentEntryIndex].quality = ParseInt(trimmedValue);
            }
            else if (m_currentRollCountContext > 0 && m_currentPoolIndex >= 0 && m_currentTableIndex >= 0 && m_currentRollRange)
            {
                LootTablePoolDefinition &pool = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex];
                if (elementName == "min")
                {
                    pool.minRolls = ParseInt(trimmedValue);
                }
                else if (elementName == "max")
                {
                    pool.maxRolls = ParseInt(trimmedValue);
                }
            }
            else if (elementName == "condition" && m_currentPoolConditionIndex >= 0 && m_currentFunctionIndex < 0 && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "conditions")
            {
                m_currentPoolConditionIndex = -1;
            }

            if (elementName == "rolls" && m_currentPoolIndex >= 0 && m_currentTableIndex >= 0)
            {
                LootTablePoolDefinition &pool = (*m_tables)[m_currentTableIndex].pools[m_currentPoolIndex];
                if (!m_currentRollRange)
                {
                    pool.rolls = ParseInt(trimmedValue);
                    pool.minRolls = pool.maxRolls = pool.rolls;
                }
                m_currentRollRange = false;
            }

            m_captureBuffer.clear();
            m_captureText = false;
            m_captureTag.clear();
            if (m_currentRollCountContext > 0)
            {
                m_currentRollCountContext = 0;
            }
            m_currentCountContext = 0;
            if (elementName == "damage")
            {
                m_currentDamageContext = false;
            }
            if (!m_captureStack.empty())
            {
                m_captureText = m_captureStack.back().first;
                m_captureTag = m_captureStack.back().second;
                m_captureStack.pop_back();
            }
        }

        if (elementName == "pool")
        {
            m_currentPoolIndex = -1;
            m_currentEntryIndex = -1;
            m_currentFunctionIndex = -1;
            m_currentPoolConditionIndex = -1;
        }
        else if (elementName == "entrie" || elementName == "entry")
        {
            m_currentEntryIndex = -1;
            m_currentFunctionIndex = -1;
            m_currentFunctionConditionIndex = -1;
        }
        else if (elementName == "function" && !m_elementStack.empty() && m_elementStack.size() >= 2 && m_elementStack[m_elementStack.size() - 2] == "functions")
        {
            m_currentFunctionIndex = -1;
            m_currentFunctionConditionIndex = -1;
        }
        else if (elementName == "loot_table")
        {
            if (m_currentTableIndex >= 0)
            {
                const LootTableDefinition &finishedTable = (*m_tables)[m_currentTableIndex];
                for (size_t p = 0; p < finishedTable.pools.size(); ++p)
                {
                    LootLog("[LootDbg] parsed pool[%zu]: entries=%zu conditions=%zu\n",
                        p, finishedTable.pools[p].entries.size(), finishedTable.pools[p].conditions.size());
                }
            }

            m_currentTableIndex = -1;
            m_currentPoolIndex = -1;
            m_currentEntryIndex = -1;
            m_currentFunctionIndex = -1;
        }

        if (!m_elementStack.empty())
        {
            m_elementStack.pop_back();
        }

        return S_OK;
    }

    HRESULT CDATABegin() override { return S_OK; }
    HRESULT CDATAData(CONST WCHAR *strCDATA, UINT CDATALen, BOOL /*bMore*/) override
    {
        if (m_captureText && strCDATA != nullptr && CDATALen > 0)
        {
            m_captureBuffer.append(ToUtf8(strCDATA, CDATALen));
        }
        return S_OK;
    }
    HRESULT CDATAEnd() override { return S_OK; }

    VOID Error(HRESULT hError, CONST CHAR *strMessage) override
    {
        app.DebugPrintf("Loot table XML parse error (%08X): %s\n", hError, strMessage ? strMessage : "(unknown)");
    }

private:
    std::vector<LootTableDefinition> *m_tables;
    std::vector<std::string> m_elementStack;
    int m_currentTableIndex = -1;
    int m_currentPoolIndex = -1;
    int m_currentEntryIndex = -1;
    int m_currentFunctionIndex = -1;
    int m_currentPoolConditionIndex = -1;
    int m_currentFunctionConditionIndex = -1;
    int m_currentCountContext = 0;
    int m_currentRollCountContext = 0;
    bool m_currentRollRange = false;
    bool m_currentDamageContext = false;
    bool m_captureText = false;
    std::string m_captureTag;
    std::string m_captureBuffer;
    std::vector<std::pair<bool, std::string>> m_captureStack;
};

int ResolveItemId(const std::string &name)
{
    // strip minecraft prefix if present
    const std::string itemName = (name.find("minecraft:") == 0) ? name.substr(10) : name;

    const int itemId = GetItemIdByName(itemName);
    if (itemId == -1)
    {
        app.DebugPrintf("[LootDbg] ResolveItemId: unrecognized item name '%s' -> 0\n", name.c_str());
        return 0;
    }

    return itemId;
}

}

LootTableManager &LootTableManager::Get()
{
    static LootTableManager instance;
    return instance;
}

bool LootTableManager::LoadFromDisk(const std::string &baseDirectory)
{
    m_tables.clear();
    m_tableIndexByPath.clear();
    m_loaded = false;
    m_rootDirectory.clear();

    // hardcoded but seems to work just fine nonetheless
    // maybe we should un-hardcode it later? idk why we would need to tho
    std::vector<std::string> candidates;
    if (!baseDirectory.empty())
    {
        candidates.push_back(baseDirectory);
    }
    candidates.push_back("./Common/Media/MediaWindows64/Structures/loot_tables");

    std::string resolvedRoot;
    for (std::vector<std::string>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
    {
        fs::path candidatePath(*it);
        if (IsValidDirectory(candidatePath))
        {
            resolvedRoot = candidatePath.string();
            break;
        }
    }

    if (resolvedRoot.empty())
    {
        app.DebugPrintf("[LootDbg] LoadFromDisk failed: no valid root directory found\n");
        return false;
    }

    std::vector<std::string> files;
    CollectXmlFiles(resolvedRoot, files);

    for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        const size_t tableCountBeforeParse = m_tables.size();
        ATG::XMLParser parser;
        XmlLootTableCallback callback(&m_tables);
        parser.RegisterSAXCallbackInterface(&callback);

        const HRESULT result = parser.ParseXMLFile(it->c_str());
        const bool tableWasAdded = m_tables.size() > tableCountBeforeParse;

        if (result == S_OK && tableWasAdded)
        {
            LootTableDefinition &table = m_tables.back();
            size_t startPos = it->find("loot_tables");
            if (startPos != std::string::npos)
            {
                table.path = NormalizeTableName(it->substr(startPos + std::string("loot_tables").length() + 1));
            }
            else
            {
                table.path = NormalizeTableName(*it);
            }
            std::replace(table.path.begin(), table.path.end(), '\\', '/');
            if (!table.path.empty() && table.path.substr(table.path.length() - 4) == ".xml")
            {
                table.path.erase(table.path.length() - 4);
            }

            m_tableIndexByPath[table.path] = m_tables.size() - 1;
        }
        else
        {
            app.DebugPrintf("LootTableManager::LoadFromDisk failed to parse '%s' (0x%08X)\n", it->c_str(), result);
        }
    }

    m_rootDirectory = resolvedRoot;
    m_loaded = !m_tables.empty();

    return m_loaded;
}

bool LootTableManager::HasLoadedTables() const
{
    return m_loaded;
}

const LootTableDefinition *LootTableManager::GetTable(const std::string &tableName) const
{
    const std::string normalized = NormalizeTableName(tableName);

    auto it = m_tableIndexByPath.find(normalized);
    if (it == m_tableIndexByPath.end())
    {
        app.DebugPrintf("[LootDbg] GetTable: no match for '%s'\n", normalized.c_str());
        return nullptr;
    }

    return &m_tables[it->second];
}

std::vector<LootTableDropResult> LootTableManager::ResolveDrops(
    const std::string &tableName,
    bool wasKilledByPlayer,
    int playerBonusLevel,
    const std::function<int(int)> &randomInt,
    bool entityIsOnFire) const
{
    LootLog("[LootDbg] ResolveDrops: table='%s' wasKilledByPlayer=%d playerBonusLevel=%d\n",
        tableName.c_str(), wasKilledByPlayer, playerBonusLevel);

    std::vector<LootTableDropResult> drops;
    const LootTableDefinition *table = GetTable(tableName);
    if (table == nullptr)
    {
        LootLog("[LootDbg] ResolveDrops: table not found, returning empty\n");
        return drops;
    }

    // some entries have type="loot_table" instead of type="item"
    const int kMaxLootTableRecursionDepth = 8;
    std::function<void(const LootTableDefinition *, int, std::vector<LootTableDropResult> &)> resolveTable =
        [&](const LootTableDefinition *currentTable, int depth, std::vector<LootTableDropResult> &outDrops)
    {
        if (currentTable == nullptr || depth > kMaxLootTableRecursionDepth)
        {
            return;
        }

        for (size_t poolIndex = 0; poolIndex < currentTable->pools.size(); ++poolIndex)
        {
            const LootTablePoolDefinition &pool = currentTable->pools[poolIndex];

            bool hasKilledByPlayerCondition = false;
            bool poolConditionsPass = true;
            for (std::vector<LootTablePoolConditionDefinition>::const_iterator condIt = pool.conditions.begin(); condIt != pool.conditions.end(); ++condIt)
            {
                LootLog("[LootDbg] pool[%zu] condition entry: '%s' chance=%f lootingMultiplier=%f\n",
                    poolIndex, condIt->conditionName.c_str(), condIt->chance, condIt->lootingMultiplier);

                if (condIt->conditionName == "killed_by_player")
                {
                    hasKilledByPlayerCondition = true;
                    if (!wasKilledByPlayer)
                    {
                        poolConditionsPass = false;
                        break;
                    }
                }
                else if (condIt->conditionName == "random_chance_with_looting")
                {
                    double adjustedChance = condIt->chance + condIt->lootingMultiplier * playerBonusLevel;
                    if (!RandomChance(adjustedChance, randomInt))
                    {
                        poolConditionsPass = false;
                        break;
                    }
                }
                else if (!condIt->conditionName.empty())
                {
                    // dont comment this debug print out pls
                    app.DebugPrintf("[LootDbg] pool[%zu] condition '%s' not implemented, skipping pool\n",
                        poolIndex, condIt->conditionName.c_str());
                    poolConditionsPass = false;
                    break;
                }
                else
                {
                    poolConditionsPass = false;
                    break;
                }
            }

            if (!poolConditionsPass)
            {
                LootLog("[LootDbg] pool[%zu]: skipped due to condition failure\n", poolIndex);
                continue;
            }

            LootLog("[LootDbg]   pool[%zu]: rolls=%d entries=%zu hasKilledByPlayerCondition=%d\n",
                poolIndex, pool.rolls, pool.entries.size(), hasKilledByPlayerCondition);

            if (pool.minRolls != pool.maxRolls)
            {
                LootLog("[LootDbg] pool[%zu]: rolls range min=%d max=%d\n", poolIndex, pool.minRolls, pool.maxRolls);
            }

            if (hasKilledByPlayerCondition && !wasKilledByPlayer)
            {
                LootLog("[LootDbg] pool[%zu]: skipped (requires killed_by_player)\n", poolIndex);
                continue;
            }

            int poolRolls = pool.rolls;
            if (pool.minRolls != pool.maxRolls)
            {
                int range = pool.maxRolls - pool.minRolls + 1;
                if (range > 0)
                {
                    poolRolls = pool.minRolls + randomInt(range);
                }
            }

            for (int roll = 0; roll < poolRolls; ++roll)
            {
                int totalWeight = 0;
                for (std::vector<LootTableEntryDefinition>::const_iterator entryIt = pool.entries.begin(); entryIt != pool.entries.end(); ++entryIt)
                {
                    totalWeight += entryIt->weight;
                }

                LootLog("[LootDbg] roll %d: totalWeight=%d\n", roll, totalWeight);

                if (totalWeight <= 0)
                {
                    LootLog("[LootDbg] roll %d: SKIPPED (totalWeight <= 0)\n", roll);
                    continue;
                }

                int choice = randomInt(totalWeight);
                LootLog("[LootDbg] roll %d: choice=%d\n", roll, choice);

                int runningWeight = 0;
                for (std::vector<LootTableEntryDefinition>::const_iterator entryIt = pool.entries.begin(); entryIt != pool.entries.end(); ++entryIt)
                {
                    runningWeight += entryIt->weight;
                    if (choice < runningWeight)
                    {
                        if (entryIt->type == "empty")
                        {
                            LootLog("[LootDbg] selected entry type=empty -> no drop\n");
                            break;
                        }

                        if (entryIt->type == "loot_table")
                        {
                            // table path, NOT an item
                            const std::string referencedTableName =
                                (entryIt->name.find("minecraft:") == 0) ? entryIt->name.substr(10) : entryIt->name;
                            const LootTableDefinition *referencedTable = GetTable(referencedTableName);
                            LootLog("[LootDbg] selected entry type=loot_table name='%s' -> %s\n",
                                referencedTableName.c_str(), referencedTable ? "resolved" : "NOT FOUND");
                            resolveTable(referencedTable, depth + 1, outDrops);
                            break;
                        }

                        LootTableDropResult result;
                        result.itemId = ResolveItemId(entryIt->name);
                        result.count = 1;

                        LootLog("[LootDbg] selected entry name='%s' weight=%d itemId=%d\n",
                            entryIt->name.c_str(), entryIt->weight, result.itemId);

                        for (std::vector<LootTableFunctionDefinition>::const_iterator functionIt = entryIt->functions.begin(); functionIt != entryIt->functions.end(); ++functionIt)
                        {
                            bool shouldApplyFunction = true;
                            for (std::vector<LootTableConditionDefinition>::const_iterator conditionIt = functionIt->conditions.begin(); conditionIt != functionIt->conditions.end(); ++conditionIt)
                            {
                                if (!EvaluateCondition(*conditionIt, entityIsOnFire))
                                {
                                    shouldApplyFunction = false;
                                    LootLog("[LootDbg] skipping function '%s' due to condition '%s'\n",
                                        functionIt->functionName.c_str(), conditionIt->conditionName.c_str());
                                    break;
                                }
                            }

                            if (!shouldApplyFunction)
                            {
                                continue;
                            }

                            if (functionIt->functionName == "set_count")
                            {
                                if (functionIt->maxCount >= functionIt->minCount)
                                {
                                    const int range = functionIt->maxCount - functionIt->minCount + 1;
                                    result.count = functionIt->minCount + (range > 0 ? randomInt(range) : 0);
                                    LootLog("[LootDbg] set_count min=%d max=%d -> count=%d\n",
                                        functionIt->minCount, functionIt->maxCount, result.count);
                                }
                            }
                            else if (functionIt->functionName == "looting_enchant")
                            {
                                if (playerBonusLevel > 0 && functionIt->maxCount >= functionIt->minCount)
                                {
                                    const int range = functionIt->maxCount - functionIt->minCount + 1;
                                    LootLog("[LootDbg] looting_enchant min=%d max=%d bonus=%d range=%d\n",
                                        functionIt->minCount, functionIt->maxCount, playerBonusLevel, range);
                                    for (int i = 0; i < playerBonusLevel; ++i)
                                    {
                                        if (range > 0)
                                        {
                                            const int lootingRoll = functionIt->minCount + randomInt(range);
                                            result.count += lootingRoll;
                                            LootLog("[LootDbg] looting roll %d: +%d -> count=%d\n", i + 1, lootingRoll, result.count);
                                        }
                                    }
                                    if (functionIt->limit > 0 && result.count > functionIt->limit)
                                    {
                                        LootLog("[LootDbg] looting_enchant capped by limit=%d (was count=%d)\n",
                                            functionIt->limit, result.count);
                                        result.count = functionIt->limit;
                                    }
                                }
                                else
                                {
                                    LootLog("[LootDbg] looting_enchant SKIPPED (playerBonusLevel=%d)\n", playerBonusLevel);
                                }
                            }
                            else if (functionIt->functionName == "furnace_smelt")
                            {
                                result.itemId = ResolveSmeltedItemId(entryIt->name, result.itemId);
                                LootLog("[LootDbg] furnace_smelt -> itemId=%d\n", result.itemId);
                            }
                            else if (functionIt->functionName == "set_data")
                            {
                                if (functionIt->maxCount >= functionIt->minCount)
                                {
                                    const int range = functionIt->maxCount - functionIt->minCount + 1;
                                    result.data = functionIt->minCount + (range > 0 ? randomInt(range) : 0);
                                    LootLog("[LootDbg] set_data min=%d max=%d -> data=%d\n",
                                        functionIt->minCount, functionIt->maxCount, result.data);
                                }
                            }
                            else if (functionIt->functionName == "set_damage")
                            {
                                if (functionIt->maxDamageFraction >= functionIt->minDamageFraction)
                                {
                                    const double range = functionIt->maxDamageFraction - functionIt->minDamageFraction;
                                    double fraction = functionIt->minDamageFraction;
                                    if (range > 0.0)
                                    {
                                        const int precision = 1000000;
                                        fraction += (range * randomInt(precision)) / static_cast<double>(precision);
                                    }
                                    result.damageFraction = fraction;
                                    LootLog("[LootDbg] set_damage min=%f max=%f -> damageFraction=%f\n",
                                        functionIt->minDamageFraction, functionIt->maxDamageFraction, result.damageFraction);
                                }
                            }
                            else if (functionIt->functionName == "set_nbt")
                            {
                                result.nbtTag = functionIt->nbtTag;
                                LootLog("[LootDbg] set_nbt -> tag='%s'\n", result.nbtTag.c_str());
                            }
                            else if (functionIt->functionName == "enchant_with_levels")
                            {
                                if (functionIt->maxCount >= functionIt->minCount)
                                {
                                    const int range = functionIt->maxCount - functionIt->minCount + 1;
                                    result.enchantLevels = functionIt->minCount + (range > 0 ? randomInt(range) : 0);
                                    result.treasureEnchant = functionIt->treasureOnly;
                                    LootLog("[LootDbg] enchant_with_levels min=%d max=%d treasure=%d -> levels=%d\n",
                                        functionIt->minCount, functionIt->maxCount, functionIt->treasureOnly, result.enchantLevels);
                                }
                            }
                            else if (functionIt->functionName == "enchant_randomly")
                            {
                                result.enchantLevels = 1 + randomInt(30);
                                LootLog("[LootDbg] enchant_randomly -> levels=%d\n", result.enchantLevels);
                            }
                        }

                        LootLog("[LootDbg] final result: itemId=%d count=%d (will drop=%d)\n",
                            result.itemId, result.count, (result.itemId != 0 && result.count > 0));

                        if (result.itemId != 0 && result.count > 0)
                        {
                            outDrops.push_back(result);
                        }
                        break;
                    }
                }
            }
        }
    };

    resolveTable(table, 0, drops);

    LootLog("[LootDbg] ResolveDrops: returning %zu drop(s)\n", drops.size());
    return drops;
}
