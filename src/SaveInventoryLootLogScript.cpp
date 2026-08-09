/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Common.h"
#include "Config.h"
#include "Creature.h"
#include "GameObject.h"
#include "GameTime.h"
#include "Group.h"
#include "LootMgr.h"
#include "Map.h"
#include "Player.h"
#include "StringFormat.h"
#include <mutex>
#include <unordered_map>

namespace
{
    // Every group member opens their own loot window and windows can be reopened, so
    // without remembering what was already logged the same drop is reported many times.
    constexpr Seconds LOGGED_LOOT_TTL = Seconds(30 * MINUTE);
}

class ModSaveInventoryLootLogScript : public PlayerScript
{
public:
    ModSaveInventoryLootLogScript() : PlayerScript("ModSaveInventoryLootLogScript", { PLAYERHOOK_ON_BEFORE_SEND_LOOT }) { }

    void OnPlayerBeforeSendLoot(Player* player, ObjectGuid lootGuid, Loot* loot) override
    {
        if (!loot || !sConfigMgr->GetOption<bool>("ModSaveInventory.Enable", true) || !sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootWindow", false))
        {
            return;
        }

        Map* map = player->GetMap();
        if (!map || !map->IsDungeon())
        {
            return;
        }

        Creature* creature = nullptr;
        std::string source;

        if (lootGuid.IsCreature())
        {
            creature = map->GetCreature(lootGuid);
            if (!creature || !(creature->IsDungeonBoss() || creature->isWorldBoss()))
            {
                return;
            }

            source = Acore::StringFormat("creature {} (entry {})", creature->GetName(), creature->GetEntry());
        }
        else if (lootGuid.IsGameObject())
        {
            GameObject* go = map->GetGameObject(lootGuid);
            source = Acore::StringFormat("gameobject {} (entry {})", go ? go->GetName() : std::string("<unknown>"), lootGuid.GetEntry());
        }
        else
        {
            return;
        }

        if (!MarkLogged(lootGuid))
        {
            return;
        }

        // Own logger so this lands in its own file instead of the "items" loot log.
        // Undefined loggers fall back to root, so a missing worldserver.conf entry
        // sends these lines to the server log rather than dropping them.
        LOG_INFO("lootwindow", "SaveInventory: Loot: {}, {}, {}, Items: {}",
            map->GetId(), map->GetInstanceId(), source, DescribeItems(loot));

        LOG_INFO("lootwindow", "SaveInventory: Players eligible: {}", DescribeEligiblePlayers(player, creature));
    }

private:
    // The raw loot, not what the client is about to render: LootView still filters by
    // conditions, quest status and round robin, so this lists more than any one player sees.
    static std::string DescribeItems(Loot const* loot)
    {
        std::string items;

        auto append = [&items](std::vector<LootItem> const& list)
        {
            for (LootItem const& item : list)
            {
                if (!items.empty())
                {
                    items += ", ";
                }

                items += std::to_string(item.itemid);
            }
        };

        append(loot->items);
        append(loot->quest_items);

        return items;
    }

    static std::string DescribeEligiblePlayers(Player* looter, Creature* creature)
    {
        // Gameobject loot has no recipient, so eligibility falls back to the opener's group.
        Group* group = creature ? creature->GetLootRecipientGroup() : looter->GetGroup();

        if (!group)
        {
            Player* owner = creature ? creature->GetLootRecipient() : looter;
            if (!owner)
            {
                owner = looter;
            }

            return Acore::StringFormat("{} ({})", owner->GetName(), owner->GetGUID().GetCounter());
        }

        std::string names;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member->GetMap() != looter->GetMap())
            {
                continue;
            }

            if (!names.empty())
            {
                names += ", ";
            }

            names += Acore::StringFormat("{} ({})", member->GetName(), member->GetGUID().GetCounter());
        }

        return names;
    }

    // Script objects are singletons shared by every player, so this can be reached from
    // more than one map update thread at a time.
    bool MarkLogged(ObjectGuid lootGuid)
    {
        std::lock_guard<std::mutex> guard(_loggedLootMutex);

        Seconds now = GameTime::GetGameTime();

        for (auto itr = _loggedLoot.begin(); itr != _loggedLoot.end();)
        {
            if (now - itr->second >= LOGGED_LOOT_TTL)
            {
                itr = _loggedLoot.erase(itr);
            }
            else
            {
                ++itr;
            }
        }

        return _loggedLoot.emplace(lootGuid, now).second;
    }

    std::unordered_map<ObjectGuid, Seconds> _loggedLoot;
    std::mutex _loggedLootMutex;
};

void AddSaveInventoryLootLogScript()
{
    new ModSaveInventoryLootLogScript();
}
