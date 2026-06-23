/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Map.h"
#include "Creature.h"
#include "Config.h"
#include "Chat.h"
#include "Tokenize.h"
#include "StringFormat.h"
#include "EventMap.h"

class ModSaveInventoryPlayerScript : public PlayerScript
{
public:
    ModSaveInventoryPlayerScript() : PlayerScript("ModSaveInventoryPlayerScript")
    {
        _checkSaveTimer = 0;
    }

    void OnPlayerLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid lootguid) override
    {
        ScheduleSaveAndLog(player, item, "looted", DescribeLootSource(lootguid));
    }

    void OnPlayerStoreNewItem(Player* player, Item* item, uint32 /*count*/) override
    {
        ScheduleSaveAndLog(player, item, "received", "");
    }

    void OnPlayerCreateItem(Player* player, Item* item, uint32 /*count*/) override
    {
        ScheduleSaveAndLog(player, item, "crafted", "");
    }

    void OnPlayerAfterStoreOrEquipNewItem(Player* player, uint32 /*vendorslot*/, Item* item, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* pVendor, VendorItem const* /*crItem*/, bool /*bStore*/) override
    {
        std::string source;
        if (pVendor)
        {
            source = Acore::StringFormat(" from vendor {} (entry {}, GUID: {})", pVendor->GetName(), pVendor->GetEntry(), pVendor->GetGUID().GetCounter());
        }

        ScheduleSaveAndLog(player, item, "purchased", source);
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (_checkSaveTimer)
        {
            if (_checkSaveTimer <= diff)
            {
                CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                player->_SaveSkills(trans);
                player->SaveInventoryAndGoldToDB(trans);
                CharacterDatabase.CommitTransaction(trans);
                _checkSaveTimer = 0;
            }
            else
            {
                _checkSaveTimer -= diff;
            }
        }
    }

    void OnPlayerLogout(Player* /*player*/) override
    {
        _checkSaveTimer = 0;
    }

    void ScheduleSaveAndLog(Player* player, Item* item, std::string_view action, std::string const& source)
    {
        if (!item || !ShouldSaveItem(item))
        {
            return;
        }

        _checkSaveTimer = sConfigMgr->GetOption<uint32>("ModSaveInventory.SaveInterval", 5000);

        if (sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootedItems", true))
        {
            std::string instanceInfo;
            if (Map* map = player->GetMap(); map && map->IsDungeon())
            {
                instanceInfo = Acore::StringFormat(" [instance map {}, save ID {}]", map->GetId(), map->GetInstanceId());
            }

            LOG_INFO("items", "SaveInventory: Player {} ({}) {} item {} (GUID: {}){}{}",
                player->GetName(), player->GetGUID().GetCounter(), action,
                item->GetEntry(), item->GetGUID().GetCounter(), source, instanceInfo);
        }
    }

    // Item and Player GUIDs carry no entry, so only creatures/gameobjects report one.
    static std::string DescribeLootSource(ObjectGuid lootguid)
    {
        if (lootguid.IsCreature())
        {
            return Acore::StringFormat(" from creature entry {} (GUID: {})", lootguid.GetEntry(), lootguid.GetCounter());
        }

        if (lootguid.IsGameObject())
        {
            return Acore::StringFormat(" from gameobject entry {} (GUID: {})", lootguid.GetEntry(), lootguid.GetCounter());
        }

        if (lootguid.IsItem())
        {
            return Acore::StringFormat(" from a container (GUID: {})", lootguid.GetCounter());
        }

        return "";
    }

    bool ShouldSaveItem(Item* item)
    {
        if (!sConfigMgr->GetOption<bool>("ModSaveInventory.Enable", true))
        {
            return false;
        }

        if (item->GetTemplate()->Quality >= sConfigMgr->GetOption<uint8>("ModSaveInventory.MinItemQuality", ITEM_QUALITY_UNCOMMON))
        {
            return true;
        }

        std::string exceptions = sConfigMgr->GetOption<std::string>("ModSaveInventory.AlwaysSaveList", "");

        std::vector<std::string_view> tokens = Acore::Tokenize(exceptions, ' ', false);

        for (auto token : tokens)
        {
            if (token.empty())
            {
                continue;
            }

            if (item->GetEntry() == Acore::StringTo<uint32>(token).value())
            {
                return true;
            }
        }

        return false;
    }

private:
    uint32 _checkSaveTimer;
};

void AddSaveInventoryScript()
{
    new ModSaveInventoryPlayerScript();
}
