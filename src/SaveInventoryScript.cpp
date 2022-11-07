/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Tokenize.h"
#include "EventMap.h"

class ModSaveInventoryPlayerScript : public PlayerScript
{
public:
    ModSaveInventoryPlayerScript() : PlayerScript("ModSaveInventoryPlayerScript")
    {
        _checkSaveTimer = 0;
    }

    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
    {
        if (!item)
        {
            return;
        }

        if (ShouldSaveItem(item))
        {
            _checkSaveTimer = sConfigMgr->GetOption<uint32>("ModSaveInventory.SaveInterval", 5000);
            if (sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootedItems", true))
            {
                LOG_INFO("items", "SaveInventory: Player {} ({}) looted item {} (GUID: {})", player->GetName(), player->GetGUID().GetCounter(), item->GetEntry(), item->GetGUID().GetCounter());
            }
        }
    }

    void OnStoreNewItem(Player* player, Item* item, uint32 /*count*/) override
    {
        if (!item)
        {
            return;
        }

        if (ShouldSaveItem(item))
        {
            _checkSaveTimer = sConfigMgr->GetOption<uint32>("ModSaveInventory.SaveInterval", 5000);
            if (sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootedItems", true))
            {
                LOG_INFO("items", "SaveInventory: Player {} ({}) looted item {} (GUID: {})", player->GetName(), player->GetGUID().GetCounter(), item->GetEntry(), item->GetGUID().GetCounter());
            }
        }
    }

    void OnCreateItem(Player* player, Item* item, uint32 /*count*/) override
    {
        if (!item)
        {
            return;
        }

        if (ShouldSaveItem(item))
        {
            _checkSaveTimer = sConfigMgr->GetOption<uint32>("ModSaveInventory.SaveInterval", 5000);
            if (sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootedItems", true))
            {
                LOG_INFO("items", "SaveInventory: Player {} ({}) looted item {} (GUID: {})", player->GetName(), player->GetGUID().GetCounter(), item->GetEntry(), item->GetGUID().GetCounter());
            }
        }
    }
    void OnAfterStoreOrEquipNewItem(Player* player, uint32 /*vendorslot*/, Item* item, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/, ItemTemplate const* /*pProto*/, Creature* /*pVendor*/, VendorItem const* /*crItem*/, bool /*bStore*/) override
    {
        if (!item)
        {
            return;
        }

        if (ShouldSaveItem(item))
        {
            _checkSaveTimer = sConfigMgr->GetOption<uint32>("ModSaveInventory.SaveInterval", 5000);
            if (sConfigMgr->GetOption<bool>("ModSaveInventory.LogLootedItems", true))
            {
                LOG_INFO("items", "SaveInventory: Player {} ({}) looted item {} (GUID: {})", player->GetName(), player->GetGUID().GetCounter(), item->GetEntry(), item->GetGUID().GetCounter());
            }
        }
    }

    void OnUpdate(Player* player, uint32 diff) override
    {
        if (_checkSaveTimer)
        {
            if (_checkSaveTimer <= diff)
            {
                CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
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

    void OnLogout(Player* /*player*/) override
    {
        _checkSaveTimer = 0;
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
