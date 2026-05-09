#include "FormCache.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // Biped slot bitmask values — match skyui.defines.Armor.PARTMASK_* exactly.
    // CommonLibSSE uses named enum values up to kEars (1<<13); slots 14-17 are
    // unnamed in CommonLib but are well-known SkyUI slots (Cloak, Backpack…).
    // ---------------------------------------------------------------------------
    namespace PartMask {
        inline constexpr std::uint32_t kHead     = 1u << 0;
        inline constexpr std::uint32_t kHair     = 1u << 1;
        inline constexpr std::uint32_t kBody     = 1u << 2;
        inline constexpr std::uint32_t kHands    = 1u << 3;
        inline constexpr std::uint32_t kForearms = 1u << 4;
        inline constexpr std::uint32_t kAmulet   = 1u << 5;
        inline constexpr std::uint32_t kRing     = 1u << 6;
        inline constexpr std::uint32_t kFeet     = 1u << 7;
        inline constexpr std::uint32_t kCalves   = 1u << 8;
        inline constexpr std::uint32_t kShield   = 1u << 9;
        inline constexpr std::uint32_t kTail     = 1u << 10;
        inline constexpr std::uint32_t kLongHair = 1u << 11;
        inline constexpr std::uint32_t kCirclet  = 1u << 12;
        inline constexpr std::uint32_t kEars     = 1u << 13;
        inline constexpr std::uint32_t kSlot14   = 1u << 14;
        inline constexpr std::uint32_t kSlot15   = 1u << 15;
        inline constexpr std::uint32_t kCloak    = 1u << 16;  // kModChestPrimary in CommonLib
        inline constexpr std::uint32_t kBackpack = 1u << 17;  // kModBack in CommonLib
    }

    // Precedence order mirrors PARTMASK_PRECEDENCE in Armor.as.
    static constexpr std::uint32_t kPartMaskPrecedence[] = {
        PartMask::kBody,    PartMask::kHair,     PartMask::kHands,
        PartMask::kForearms,PartMask::kFeet,     PartMask::kCalves,
        PartMask::kShield,  PartMask::kAmulet,   PartMask::kRing,
        PartMask::kLongHair,PartMask::kEars,     PartMask::kHead,
        PartMask::kCirclet, PartMask::kTail,     PartMask::kSlot14,
        PartMask::kSlot15,  PartMask::kCloak,    PartMask::kBackpack,
    };

    // ---------------------------------------------------------------------------
    // Hardcoded base-game FormIDs for pickaxes and wood axes.
    // Mirrors processWeaponBaseId() in InventoryDataSetter.as.
    // Only base-game (plugin index 0x00) forms need this treatment; DLC2 pickaxes
    // (index 0x04) are handled there too but use the same kPickaxe constant.
    // ---------------------------------------------------------------------------
    namespace WeaponFormID {
        // Pickaxes (Skyrim.esm)
        inline constexpr RE::FormID kPickaxe               = 0x000E3C16;
        inline constexpr RE::FormID kPickaxeRockSplinter   = 0x0006A707;
        inline constexpr RE::FormID kPickaxeVolunruud      = 0x001019D4;
        // Wood axes (Skyrim.esm)
        inline constexpr RE::FormID kWoodAxe               = 0x0002F2F4;
        inline constexpr RE::FormID kWoodAxePoacherHalted  = 0x000AE086;
        // Dragonborn DLC pickaxes (plugin index 0x04)
        inline constexpr RE::FormID kDLC2Pickaxe1          = 0x04027113;
        inline constexpr RE::FormID kDLC2Pickaxe2          = 0x0402711C;
        inline constexpr RE::FormID kDLC2Pickaxe3          = 0x04027120;
        // Bows with Wood material (Skyrim.esm)
        inline constexpr RE::FormID kLongbow               = 0x0003B562;
        inline constexpr RE::FormID kHuntingBow            = 0x00013985;
        inline constexpr RE::FormID kDravinsbow            = 0x0006B9AD;
    }

    // ---------------------------------------------------------------------------
    // FormCache — singleton
    // ---------------------------------------------------------------------------
    FormCache* FormCache::GetSingleton() {
        static FormCache instance;
        return &instance;
    }

    void FormCache::Populate(RE::Actor* a_actor) {
        if (!a_actor) return;

        auto inventory = a_actor->GetInventory();

        // Build entries for every form not yet in the cache.
        std::unordered_map<RE::FormID, CachedItemData> newEntries;
        {
            std::shared_lock lock(_mutex);
            for (auto& [item, data] : inventory) {
                if (!item) continue;
                if (_cache.contains(item->GetFormID())) continue;
                newEntries.emplace(item->GetFormID(), BuildItemData(item));
            }
        }

        if (newEntries.empty()) return;

        const std::size_t added = newEntries.size();
        {
            std::unique_lock lock(_mutex);
            _cache.merge(newEntries);
        }

        logger::debug("FormCache: populated {} new entries (total {})",
                      added, _cache.size());
    }

    void FormCache::Clear() {
        std::unique_lock lock(_mutex);
        _cache.clear();
        logger::debug("FormCache: cleared");
    }

    const CachedItemData* FormCache::Get(RE::FormID a_formID) const {
        std::shared_lock lock(_mutex);
        auto it = _cache.find(a_formID);
        return (it != _cache.end()) ? &it->second : nullptr;
    }

    // ---------------------------------------------------------------------------
    // BuildItemData — dispatch to type-specific builders
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildItemData(RE::TESBoundObject* a_item) {
        if (!a_item) return {};

        if (auto* weap = a_item->As<RE::TESObjectWEAP>())    return BuildWeaponData(weap);
        if (auto* armo = a_item->As<RE::TESObjectARMO>())    return BuildArmorData(armo);
        if (auto* ammo = a_item->As<RE::TESAmmo>())          return BuildAmmoData(ammo);
        if (auto* book = a_item->As<RE::TESObjectBOOK>())    return BuildBookData(book);
        if (auto* alch = a_item->As<RE::AlchemyItem>())      return BuildAlchemyData(alch);
        if (auto* ingr = a_item->As<RE::IngredientItem>())   return BuildIngredientData(ingr);
        if (auto* key  = a_item->As<RE::TESKey>())           return BuildKeyData(key);
        if (auto* soul = a_item->As<RE::TESSoulGem>())       return BuildSoulGemData(soul);
        if (auto* scrl = a_item->As<RE::ScrollItem>())       return BuildScrollData(scrl);
        if (auto* misc = a_item->As<RE::TESObjectMISC>())    return BuildMiscData(misc);

        return {};
    }

    // ---------------------------------------------------------------------------
    // Weapons
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildWeaponData(RE::TESObjectWEAP* a_weapon) {
        CachedItemData d;

        // Determine subType from weapon animation type.
        const auto formID = a_weapon->GetFormID();
        switch (a_weapon->GetWeaponType()) {
            case RE::WEAPON_TYPE::kHandToHandMelee:
                d.subType = WeaponSubType::kMelee;
                break;
            case RE::WEAPON_TYPE::kOneHandSword:
                if (a_weapon->HasKeywordString("ccBGSSSE001_FishingPoleKW"))
                    d.subType = WeaponSubType::kFishingRod;
                else
                    d.subType = WeaponSubType::kSword;
                break;
            case RE::WEAPON_TYPE::kOneHandDagger:
                d.subType = WeaponSubType::kDagger;
                break;
            case RE::WEAPON_TYPE::kOneHandAxe:
                d.subType = WeaponSubType::kWarAxe;
                break;
            case RE::WEAPON_TYPE::kOneHandMace:
                d.subType = WeaponSubType::kMace;
                break;
            case RE::WEAPON_TYPE::kTwoHandSword:
                d.subType = WeaponSubType::kGreatsword;
                break;
            case RE::WEAPON_TYPE::kTwoHandAxe:
                d.subType = a_weapon->HasKeywordString("WeapTypeWarhammer")
                                ? WeaponSubType::kWarhammer
                                : WeaponSubType::kBattleAxe;
                break;
            case RE::WEAPON_TYPE::kBow:
                d.subType = WeaponSubType::kBow;
                break;
            case RE::WEAPON_TYPE::kStaff:
                d.subType = WeaponSubType::kStaff;
                break;
            case RE::WEAPON_TYPE::kCrossbow:
                d.subType = WeaponSubType::kCrossbow;
                break;
            default:
                break;
        }

        // Override subType for hardcoded base-game tool weapons.
        switch (formID) {
            case WeaponFormID::kPickaxe:
            case WeaponFormID::kPickaxeRockSplinter:
            case WeaponFormID::kPickaxeVolunruud:
            case WeaponFormID::kDLC2Pickaxe1:
            case WeaponFormID::kDLC2Pickaxe2:
            case WeaponFormID::kDLC2Pickaxe3:
                d.subType = WeaponSubType::kPickaxe;
                break;
            case WeaponFormID::kWoodAxe:
            case WeaponFormID::kWoodAxePoacherHalted:
                d.subType = WeaponSubType::kWoodAxe;
                break;
            default:
                break;
        }

        // Material from keywords.
        d.material = GetMaterial(a_weapon);

        // Hardcoded wood material for unkeyworted hunting bows.
        if (d.material == MaterialType::kNone) {
            switch (formID) {
                case WeaponFormID::kLongbow:
                case WeaponFormID::kHuntingBow:
                case WeaponFormID::kDravinsbow:
                    d.material = MaterialType::kWood;
                    break;
                default:
                    break;
            }
        }

        return d;
    }

    // ---------------------------------------------------------------------------
    // Armor
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildArmorData(RE::TESObjectARMO* a_armor) {
        CachedItemData d;

        // weightClass from engine armor type, then refined for clothing/jewelry.
        const auto armorType = a_armor->GetArmorType();
        if (armorType == RE::BGSBipedObjectForm::ArmorType::kLightArmor)
            d.weightClass = ArmorWeightClass::kLight;
        else if (armorType == RE::BGSBipedObjectForm::ArmorType::kHeavyArmor)
            d.weightClass = ArmorWeightClass::kHeavy;
        else
            d.weightClass = ArmorWeightClass::kNone;  // will be refined below

        // partMask → mainPartMask → subType.
        const auto slotMask = a_armor->GetSlotMask().underlying();
        d.mainPartMask = GetMainPartMask(slotMask);
        if (d.mainPartMask > 0)
            d.subType = ArmorSubTypeFromPartMask(d.mainPartMask);

        // Clothing/jewelry disambiguation for pieces that have WEIGHT_NONE.
        d.weightClass = ResolveWeightClass(d.weightClass, d.mainPartMask, a_armor);

        // Material from keywords.
        d.material = GetMaterial(a_armor);

        return d;
    }

    // ---------------------------------------------------------------------------
    // Ammo
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildAmmoData(RE::TESAmmo* a_ammo) {
        CachedItemData d;
        d.subType = a_ammo->IsBolt() ? AmmoSubType::kBolt : AmmoSubType::kArrow;
        d.material = GetMaterial(a_ammo->AsKeywordForm());
        return d;
    }

    // ---------------------------------------------------------------------------
    // Books — mirrors processBookType()
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildBookData(RE::TESObjectBOOK* a_book) {
        CachedItemData d;

        if (a_book->data.type.any(RE::OBJ_BOOK::Type::kNoteScroll)) {
            d.subType = BookSubType::kNote;
            return d;
        }

        d.subType = BookSubType::kBook;

        if (a_book->HasKeywordString("VendorItemSpellTome"))
            d.subType = BookSubType::kSpellTome;
        else if (a_book->HasKeywordString("VendorItemRecipe"))
            d.subType = BookSubType::kRecipe;

        return d;
    }

    // ---------------------------------------------------------------------------
    // Potions / Alchemy — mirrors processPotionType()
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildAlchemyData(RE::AlchemyItem* a_alchemy) {
        CachedItemData d;

        if (a_alchemy->IsFood()) {
            // Distinguish drinkable food by the ITMPotionUse sound (0x000B6435).
            // If it uses that sound it's a drink, otherwise generic food.
            constexpr RE::FormID kITMPotionUse = 0x000B6435;
            bool isDrink = false;
            const auto& alchData = a_alchemy->data;
            if (alchData.consumptionSound) {
                isDrink = (alchData.consumptionSound->GetFormID() == kITMPotionUse);
            }            d.subType = isDrink ? PotionSubType::kDrink : PotionSubType::kFood;
            return d;
        }

        if (a_alchemy->IsPoison()) {
            d.subType = PotionSubType::kPoison;
            return d;
        }

        // Generic potion — identify by primary actor value of the costliest effect.
        d.subType = PotionSubType::kPotion;  // default

        // Walk effects to find the highest-cost one (matches how the game picks
        // the actorValue it sends to Scaleform).
        float      bestCost = -1.f;
        RE::ActorValue bestAV = RE::ActorValue::kNone;
        for (auto* effect : a_alchemy->effects) {
            if (!effect || !effect->baseEffect) continue;
            const float cost = effect->baseEffect->data.baseCost;
            if (cost > bestCost) {
                bestCost = cost;
                bestAV   = effect->baseEffect->data.primaryAV;
            }
        }

        // Map ActorValue → PotionSubType, mirroring processPotionType() in AS3.
        switch (bestAV) {
            case RE::ActorValue::kHealth:        d.subType = PotionSubType::kHealth;      break;
            case RE::ActorValue::kHealRate:      d.subType = PotionSubType::kHealRate;    break;
            case RE::ActorValue::kMagicka:       d.subType = PotionSubType::kMagicka;     break;
            case RE::ActorValue::kMagickaRate:   d.subType = PotionSubType::kMagickaRate; break;
            case RE::ActorValue::kStamina:       d.subType = PotionSubType::kStamina;     break;
            case RE::ActorValue::kStaminaRate:   d.subType = PotionSubType::kStaminaRate; break;
            default:                             break;
        }

        return d;
    }

    // ---------------------------------------------------------------------------
    // Ingredient — subType is always undefined in AS3; nothing to compute
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildIngredientData(RE::IngredientItem*) {
        return {};  // subType -1 ≡ AS3 undefined
    }

    // ---------------------------------------------------------------------------
    // Key — subType is always undefined in AS3
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildKeyData(RE::TESKey*) {
        return {};
    }

    // ---------------------------------------------------------------------------
    // Soul gem — subType is set from the gem's maximum capacity (static), which
    // maps directly to the SOULGEM_* constants.  The fill state (soulSize/status)
    // changes at runtime and must NOT be cached — processSoulGemStatus() is still
    // called in the fast path to compute status from the live gemSize/soulSize.
    // ---------------------------------------------------------------------------

    // FormID table for soul gem special cases (mirrors processSoulGemBaseId() in AS2).
    static std::optional<std::int32_t> GetSoulGemSubTypeFromFormID(RE::FormID a_formID) {
        static const auto s_table = []() {
            std::unordered_map<RE::FormID, std::int32_t> table;
            auto* dh = RE::TESDataHandler::GetSingleton();
            if (!dh) return table;

            auto add = [&](std::string_view plugin, RE::FormID localId, std::int32_t subType) {
                if (auto* form = dh->LookupForm(localId, plugin))
                    table.emplace(form->GetFormID(), subType);
            };

            // Azura's Star and The Black Star → SOULGEM_AZURA
            add("Skyrim.esm", 0x063B27, SoulGemSubType::kAzura);
            add("Skyrim.esm", 0x063B29, SoulGemSubType::kAzura);

            // CC Soul Tomato skipped — CC plugin filenames are not reliably stable.

            return table;
        }();

        const auto it = s_table.find(a_formID);
        return (it != s_table.end()) ? std::optional(it->second) : std::nullopt;
    }

    CachedItemData FormCache::BuildSoulGemData(RE::TESSoulGem* a_soulGem) {
        CachedItemData d;

        // subType = maximum soul capacity — maps directly to SOULGEM_PETTY..SOULGEM_GRAND.
        const auto capacity = static_cast<std::int32_t>(a_soulGem->GetMaximumCapacity());
        if (capacity >= SoulGemSubType::kPetty && capacity <= SoulGemSubType::kGrand)
            d.subType = capacity;

        // FormID-based overrides (Azura's Star, Black Star).
        if (auto override = GetSoulGemSubTypeFromFormID(a_soulGem->GetFormID()))
            d.subType = *override;

        return d;
    }

    // ---------------------------------------------------------------------------
    // Scroll — subType undefined in AS3
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildScrollData(RE::ScrollItem*) {
        return {};
    }

    // ---------------------------------------------------------------------------
    // Misc item FormID lookup table — mirrors processMiscBaseId() in AS2.
    //
    // Built once on first call using TESDataHandler::LookupForm(localId, plugin),
    // which resolves FormIDs against the actual load order.  Plugins that are not
    // installed simply contribute no entries (LookupForm returns nullptr).
    //
    // localId is the plugin-local FormID (lower 24 bits only; no plugin-index byte).
    // ---------------------------------------------------------------------------
    struct MiscOverride {
        std::int32_t subType;
        const char*  displayKey{ nullptr };
    };

    static std::optional<MiscOverride> GetMiscSubTypeFromFormID(RE::FormID a_formID) {
        static const auto s_table = []() {
            std::unordered_map<RE::FormID, MiscOverride> table;
            auto* dh = RE::TESDataHandler::GetSingleton();
            if (!dh) return table;

            auto add = [&](std::string_view plugin, RE::FormID localId, std::int32_t subType,
                           const char* displayKey = nullptr) {
                if (auto* form = dh->LookupForm(localId, plugin))
                    table.emplace(form->GetFormID(), MiscOverride{subType, displayKey});
            };

            // --- Skyrim.esm ---
            // Gems
            add("Skyrim.esm", 0x06851E, MiscSubType::kGem);  // Flawless Amethyst
            add("Skyrim.esm", 0x01994F, MiscSubType::kGem);
            add("Skyrim.esm", 0x059654, MiscSubType::kGem);
            add("Skyrim.esm", 0x09DFBB, MiscSubType::kGem);
            add("Skyrim.esm", 0x09F7A6, MiscSubType::kGem);
            // Dragon claws
            add("Skyrim.esm", 0x04B56C, MiscSubType::kDragonclaw);  // Ruby
            add("Skyrim.esm", 0x0AB7BB, MiscSubType::kDragonclaw);  // Ivory
            add("Skyrim.esm", 0x07C260, MiscSubType::kDragonclaw);  // Glass
            add("Skyrim.esm", 0x05AF48, MiscSubType::kDragonclaw);  // Ebony
            add("Skyrim.esm", 0x0ED417, MiscSubType::kDragonclaw);  // Emerald
            add("Skyrim.esm", 0x0AB375, MiscSubType::kDragonclaw);  // Diamond
            add("Skyrim.esm", 0x08CDFA, MiscSubType::kDragonclaw);  // Iron
            add("Skyrim.esm", 0x0B634C, MiscSubType::kDragonclaw);  // Coral
            add("Skyrim.esm", 0x0999E7, MiscSubType::kDragonclaw);  // Golden (E3)
            add("Skyrim.esm", 0x0663D7, MiscSubType::kDragonclaw);  // Sapphire
            add("Skyrim.esm", 0x039647, MiscSubType::kDragonclaw);  // Golden (MS13)
            // Remains
            add("Skyrim.esm", 0x075868, MiscSubType::kRemains);
            add("Skyrim.esm", 0x0F6767, MiscSubType::kRemains);
            add("Skyrim.esm", 0x0AADB6, MiscSubType::kRemains);
            add("Skyrim.esm", 0x0AADB7, MiscSubType::kRemains);
            add("Skyrim.esm", 0x04286C, MiscSubType::kRemains);
            // Special items
            add("Skyrim.esm", 0x00000A, MiscSubType::kLockpick    );
            add("Skyrim.esm", 0x00000F, MiscSubType::kGold        );
            add("Skyrim.esm", 0x0DB5D2, MiscSubType::kLeather     );
            add("Skyrim.esm", 0x0800E4, MiscSubType::kLeatherStrips);
            add("Skyrim.esm", 0x03AD57, MiscSubType::kNetchLeather);
            // Broken weapons (31)
            add("Skyrim.esm", 0x0E72AA, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72AC, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72B0, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72AE, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72A6, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72A8, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72A0, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E729E, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E729A, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E729C, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E7296, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E7298, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72A4, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0E72A2, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064283, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064285, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064287, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064289, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x06428B, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x06428E, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064290, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064292, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064294, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064296, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x064298, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x06E806, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0DB351, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0240D3, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0240D4, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0240D5, MiscSubType::kBrokenWeapon);
            add("Skyrim.esm", 0x0240D6, MiscSubType::kBrokenWeapon);
            // Dwarven scrap (12)
            add("Skyrim.esm", 0x0C886C, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8878, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8864, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8872, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8866, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8874, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C886A, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0AEBF1, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8861, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8868, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C886E, MiscSubType::kDwarvenScrap);
            add("Skyrim.esm", 0x0C8870, MiscSubType::kDwarvenScrap);
            // Instruments (9)
            add("Skyrim.esm", 0x0DABA9, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x0DABA7, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x105177, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x03292F, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x0200BA, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x0DABAB, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x0200B6, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x105109, MiscSubType::kInstrument);
            add("Skyrim.esm", 0x0E77BB, MiscSubType::kInstrument);
            // Bug jars (5)
            add("Skyrim.esm", 0x0B08C7, MiscSubType::kBugJar);
            add("Skyrim.esm", 0x0FBC3A, MiscSubType::kBugJar);
            add("Skyrim.esm", 0x0FBC3B, MiscSubType::kBugJar);
            add("Skyrim.esm", 0x0FBC3C, MiscSubType::kBugJar);
            add("Skyrim.esm", 0x0FBC3D, MiscSubType::kBugJar);
            // Maps
            add("Skyrim.esm", 0x060CC2, MiscSubType::kMap);
            add("Skyrim.esm", 0x0BBCD5, MiscSubType::kMap);
            // Daedric artifacts
            add("Skyrim.esm", 0x028AD7, MiscSubType::kArtifact);  // Azura's Star
            add("Skyrim.esm", 0x02C259, MiscSubType::kArtifact);
            add("Skyrim.esm", 0x02C25A, MiscSubType::kArtifact);
            // Misc re-classified items
            add("Skyrim.esm", 0x0C4F2E, MiscSubType::kPotion);
            add("Skyrim.esm", 0x02BAAB, MiscSubType::kPoison);
            add("Skyrim.esm", 0x0457AB, MiscSubType::kScroll);
            add("Skyrim.esm", 0x0DC530, MiscSubType::kScroll);
            add("Skyrim.esm", 0x0DC52E, MiscSubType::kScroll);
            add("Skyrim.esm", 0x0F1491, MiscSubType::kBook  );
            add("Skyrim.esm", 0x0CE70B, MiscSubType::kBook  );
            add("Skyrim.esm", 0x0E4897, MiscSubType::kBook  );
            add("Skyrim.esm", 0x0E3CB7, MiscSubType::kBook  );
            add("Skyrim.esm", 0x01CB34, MiscSubType::kRing  );
            add("Skyrim.esm", 0x0DA732, MiscSubType::kRing  );
            add("Skyrim.esm", 0x0DA733, MiscSubType::kRing  );
            add("Skyrim.esm", 0x0DA734, MiscSubType::kRing  );
            add("Skyrim.esm", 0x0DA735, MiscSubType::kRing  );
            // Ores (10)
            add("Skyrim.esm", 0x05ACDB, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACDC, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACDE, MiscSubType::kOre);
            add("Skyrim.esm", 0x071CF3, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACE1, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACE0, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACDD, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACE2, MiscSubType::kOre);
            add("Skyrim.esm", 0x05B2DF, MiscSubType::kOre);
            add("Skyrim.esm", 0x05ACDF, MiscSubType::kOre);

            // --- Update.esm ---
            add("Update.esm", 0x0030C9, MiscSubType::kHorseTack);
            add("Update.esm", 0x0030CA, MiscSubType::kHorseTack);

            // --- Dawnguard.esm ---
            // Gems
            add("Dawnguard.esm", 0x012F97, MiscSubType::kGem);
            add("Dawnguard.esm", 0x012FC3, MiscSubType::kGem);
            add("Dawnguard.esm", 0x019ABB, MiscSubType::kGem);
            add("Dawnguard.esm", 0x019ABC, MiscSubType::kGem);
            add("Dawnguard.esm", 0x019ABD, MiscSubType::kGem);
            // Remains
            add("Dawnguard.esm", 0x002993, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x002994, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x011CF7, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x005704, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x005705, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x005706, MiscSubType::kRemains);
            add("Dawnguard.esm", 0x005707, MiscSubType::kRemains);
            // Netch leather (chitin)
            add("Dawnguard.esm", 0x0195AA, MiscSubType::kNetchLeather);

            // --- HearthFires.esm --- house parts (10)
            // These items are also caught by BYOH crafting keywords (phase 1 → "$House Part"),
            // but processMiscBaseId() in AS2 overrides their display to "$BuildingMaterial".
            // The displayKey here propagates that override through the fast path.
            add("HearthFires.esm", 0x003043, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x003035, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x005A69, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x003011, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x00303F, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x003012, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x00300E, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x00300F, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x00306C, MiscSubType::kHousePart, "$BuildingMaterial");
            add("HearthFires.esm", 0x005A68, MiscSubType::kHousePart, "$BuildingMaterial");

            // --- Dragonborn.esm ---
            // Dragon claws
            add("Dragonborn.esm", 0x01CAC0, MiscSubType::kDragonclaw);
            add("Dragonborn.esm", 0x01CAC1, MiscSubType::kDragonclaw);
            // Gems
            add("Dragonborn.esm", 0x02145A, MiscSubType::kGem);
            add("Dragonborn.esm", 0x03166F, MiscSubType::kGem);
            add("Dragonborn.esm", 0x031670, MiscSubType::kGem);
            add("Dragonborn.esm", 0x031671, MiscSubType::kGem);
            add("Dragonborn.esm", 0x031672, MiscSubType::kGem);
            // Netch leather (chitin + netch)
            add("Dragonborn.esm", 0x02B04E, MiscSubType::kNetchLeather);
            add("Dragonborn.esm", 0x01CD7C, MiscSubType::kNetchLeather);
            // Other
            add("Dragonborn.esm", 0x0247F9, MiscSubType::kTrollskull );
            add("Dragonborn.esm", 0x017719, MiscSubType::kScrollSpider);
            add("Dragonborn.esm", 0x01771F, MiscSubType::kScrollSpider);
            add("Dragonborn.esm", 0x02BAAE, MiscSubType::kMap        );
            add("Dragonborn.esm", 0x01AAD6, MiscSubType::kIngredient );
            // Ores (3)
            add("Dragonborn.esm", 0x02B06B, MiscSubType::kOre);
            add("Dragonborn.esm", 0x017749, MiscSubType::kOre);
            add("Dragonborn.esm", 0x0195A9, MiscSubType::kOre);

            // --- CC plugins ---
            // CC items that have proper vendor keywords are classified correctly
            // by the keyword phase above; specific CC items without keywords are
            // not covered here since CC plugin filenames and local IDs vary
            // between game versions and are not reliably stable.

            return table;
        }();

        const auto it = s_table.find(a_formID);
        return (it != s_table.end()) ? std::optional(it->second) : std::nullopt;
    }

    void FormCache::Initialize() {
        // Trigger one-time static initialization of all FormID lookup tables on the
        // main thread (kDataLoaded), before Populate() can be called concurrently.
        GetMiscSubTypeFromFormID(0);
        GetSoulGemSubTypeFromFormID(0);
    }

    // ---------------------------------------------------------------------------
    // Misc — two-phase classification mirroring InventoryDataSetter.as:
    //   Phase 1 (processMiscType)   — keyword-based, covers most items.
    //   Phase 2 (processMiscBaseId) — FormID overrides, refines edge cases
    //                                 (e.g. distinguishes ore from ingot, adds
    //                                 gold/lockpick/claws that lack keywords).
    // ---------------------------------------------------------------------------
    CachedItemData FormCache::BuildMiscData(RE::TESObjectMISC* a_misc) {
        CachedItemData d;

        // Phase 1: keyword-based classification.
        // Priority order mirrors processMiscType() in AS2 exactly.
        auto has = [&](std::initializer_list<std::string_view> kwList) {
            for (auto kw : kwList)
                if (a_misc->HasKeywordString(kw)) return true;
            return false;
        };

        if (has({"BYOHAdoptionClothesKeyword"}))
            d.subType = MiscSubType::kChildrensClothes;
        else if (has({"BYOHAdoptionToyKeyword"}))
            d.subType = MiscSubType::kToy;
        else if (has({"BYOHHouseCraftingCategoryWeaponRacks",
                      "BYOHHouseCraftingCategoryShelf",
                      "BYOHHouseCraftingCategoryFurniture",
                      "BYOHHouseCraftingCategoryExterior",
                      "BYOHHouseCraftingCategoryContainers",
                      "BYOHHouseCraftingCategoryBuilding",
                      "BYOHHouseCraftingCategorySmithing"}))
            d.subType = MiscSubType::kHousePart;
        else if (has({"VendorItemDaedricArtifact"}))
            d.subType = MiscSubType::kArtifact;
        else if (has({"VendorItemGem"}))
            d.subType = MiscSubType::kGem;
        else if (has({"VendorItemAnimalHide"}))
            d.subType = MiscSubType::kHide;
        else if (has({"VendorItemTool"}))
            d.subType = MiscSubType::kTool;
        else if (has({"VendorItemAnimalPart"}))
            d.subType = MiscSubType::kRemains;
        else if (has({"VendorItemOreIngot"}))
            d.subType = MiscSubType::kIngot;
        else if (has({"VendorItemFireword"}))  // note: original AS2 has this typo
            d.subType = MiscSubType::kFirewood;
        else if (has({"VendorItemClutter"}))
            d.subType = MiscSubType::kClutter;

        // Phase 2: FormID-based overrides.
        // Applied on top of phase 1 result, mirroring processMiscBaseId() in AS2.
        if (auto override = GetMiscSubTypeFromFormID(a_misc->GetFormID())) {
            d.subType          = override->subType;
            d.subTypeDisplayKey = override->displayKey;  // null for most items
        }

        return d;
    }

    // ---------------------------------------------------------------------------
    // GetMaterial — keyword-based material lookup.
    // Mirrors processMaterialKeywords() in InventoryDataSetter.as exactly,
    // preserving the same priority order so results match the AS3 output.
    // ---------------------------------------------------------------------------
    std::int32_t FormCache::GetMaterial(RE::BGSKeywordForm* a_kwForm) {
        if (!a_kwForm) return MaterialType::kNone;

        // Helper lambda: true if the form has any of the listed keyword editor IDs.
        auto has = [&](std::initializer_list<std::string_view> kwList) {
            for (auto kw : kwList)
                if (a_kwForm->HasKeywordString(kw)) return true;
            return false;
        };

        if (has({"ArmorMaterialDaedric","WeapMaterialDaedric",
                 "ccBGSSSE025_ArmorMaterialDark","ccBGSSSE025_WeapMaterialDark",
                 "ccBGSSSE025_ArmorMaterialGolden","ccBGSSSE025_WeapMaterialGolden"}))
            return MaterialType::kDaedric;

        if (has({"ArmorMaterialDragonplate","ArmorMaterialDragonscale",
                 "DLC1WeapMaterialDragonbone"}))
            return MaterialType::kDragon;

        if (has({"ArmorMaterialDwarven","WeapMaterialDwarven"}))
            return MaterialType::kDwarven;

        if (has({"ArmorMaterialEbony","WeapMaterialEbony"}))
            return MaterialType::kEbony;

        if (has({"ArmorMaterialElven","WeapMaterialElven","ArmorMaterialElvenGilded"}))
            return MaterialType::kElven;

        if (has({"ArmorMaterialGlass","WeapMaterialGlass"}))
            return MaterialType::kGlass;

        if (has({"ArmorMaterialHide","ArmorMaterialScaled"}))
            return MaterialType::kHide;

        if (has({"ArmorMaterialStormcloak","ArmorMaterialBearStormcloak"}))
            return MaterialType::kStormcloak;

        if (has({"ArmorMaterialImperialHeavy","ArmorMaterialImperialLight",
                 "WeapMaterialImperial","ArmorMaterialImperialStudded",
                 "ArmorMaterialStudded"}))
            return MaterialType::kImperial;

        if (has({"ArmorMaterialIron","WeapMaterialIron","ArmorMaterialIronBanded"}))
            return MaterialType::kIron;

        if (has({"ArmorMaterialLeather"}))
            return MaterialType::kLeather;

        if (has({"ArmorMaterialOrcish","WeapMaterialOrcish",
                 "ccBGSSSE055_ArmorMaterialOrcishLight"}))
            return MaterialType::kOrcish;

        if (has({"ArmorMaterialSteel","WeapMaterialSteel","ArmorMaterialSteelPlate",
                 "WeapMaterialDraugr","WeapMaterialDraugrHoned"}))
            return MaterialType::kSteel;

        if (has({"WeapMaterialSilver"}))
            return MaterialType::kSilver;

        if (has({"ArmorMaterialFalmer","DLC1ArmorMaterialFalmerHardened",
                 "DLC1ArmorMaterielFalmerHeavy","DLC1ArmorMaterielFalmerHeavyOriginal",
                 "WeapMaterialFalmer","WeapMaterialFalmerHoned"}))
            return MaterialType::kFalmer;

        if (has({"DLC2ArmorMaterialBonemoldHeavy","DLC2ArmorMaterialBonemoldLight"}))
            return MaterialType::kBonemold;

        if (has({"DLC2ArmorMaterialChitinHeavy","DLC2ArmorMaterialChitinLight",
                 "DLC2ArmorMaterialMoragTong"}))
            return MaterialType::kChitin;

        if (has({"DLC2ArmorMaterialNordicHeavy","DLC2ArmorMaterialNordicLight",
                 "DLC2WeaponMaterialNordic"}))
            return MaterialType::kNordic;

        if (has({"DLC2ArmorMaterialStalhrimHeavy","DLC2ArmorMaterialStalhrimLight",
                 "DLC2WeaponMaterialStalhrim"}))
            return MaterialType::kStalhrim;

        if (has({"ccASVSSE001_ArmorOrdinator","ccASVSSE001_ArmorOrdinatorIndoril"}))
            return MaterialType::kOrdinator;

        if (has({"ccBGSSSE025_ArmorMaterialAmber","ccBGSSSE025_WeapMaterialAmber"}))
            return MaterialType::kAmber;

        if (has({"ccBGSSSE025_ArmorMaterialMadness","ccBGSSSE025_WeapMaterialMadness"}))
            return MaterialType::kMadness;

        if (has({"WeapMaterialWood"}))
            return MaterialType::kWood;

        return MaterialType::kNone;
    }

    // ---------------------------------------------------------------------------
    // GetMainPartMask — walk the PARTMASK_PRECEDENCE list and return the first
    // set bit.  Returns 0 if partMask is empty.
    // ---------------------------------------------------------------------------
    std::uint32_t FormCache::GetMainPartMask(std::uint32_t a_partMask) {
        for (auto mask : kPartMaskPrecedence)
            if (a_partMask & mask) return mask;
        return 0;
    }

    // ---------------------------------------------------------------------------
    // ArmorSubTypeFromPartMask — mirrors the switch in processArmorPartMask().
    // ---------------------------------------------------------------------------
    std::int32_t FormCache::ArmorSubTypeFromPartMask(std::uint32_t a_mainPartMask) {
        switch (a_mainPartMask) {
            case PartMask::kHead:     return ArmorSubType::kHead;
            case PartMask::kHair:     return ArmorSubType::kHair;
            case PartMask::kLongHair: return ArmorSubType::kLongHair;
            case PartMask::kBody:     return ArmorSubType::kBody;
            case PartMask::kForearms: return ArmorSubType::kForearms;
            case PartMask::kHands:    return ArmorSubType::kHands;
            case PartMask::kShield:   return ArmorSubType::kShield;
            case PartMask::kCalves:   return ArmorSubType::kCalves;
            case PartMask::kFeet:     return ArmorSubType::kFeet;
            case PartMask::kCirclet:  return ArmorSubType::kCirclet;
            case PartMask::kAmulet:   return ArmorSubType::kAmulet;
            case PartMask::kEars:     return ArmorSubType::kEars;
            case PartMask::kRing:     return ArmorSubType::kRing;
            case PartMask::kTail:     return ArmorSubType::kTail;
            case PartMask::kCloak:    return ArmorSubType::kCloak;
            case PartMask::kBackpack: return ArmorSubType::kBackpack;
            default:
                // Unnamed slots — AS3 falls back to the mask value itself.
                return static_cast<std::int32_t>(a_mainPartMask);
        }
    }

    // ---------------------------------------------------------------------------
    // ResolveWeightClass — clothing/jewelry disambiguation for WEIGHT_NONE pieces.
    // Mirrors processArmorClass() + processArmorOther() in InventoryDataSetter.as.
    // ---------------------------------------------------------------------------
    std::int32_t FormCache::ResolveWeightClass(std::int32_t a_engineWeightClass,
                                               std::uint32_t a_mainPartMask,
                                               RE::BGSKeywordForm* a_kwForm) {
        if (a_engineWeightClass != ArmorWeightClass::kNone)
            return a_engineWeightClass;

        // Check VendorItem keywords first (processArmorClass path).
        if (a_kwForm) {
            if (a_kwForm->HasKeywordString("VendorItemClothing"))
                return ArmorWeightClass::kClothing;
            if (a_kwForm->HasKeywordString("VendorItemJewelry"))
                return ArmorWeightClass::kJewelry;
        }

        // Fall back to slot-based classification (processArmorOther path).
        switch (a_mainPartMask) {
            case PartMask::kHead:
            case PartMask::kHair:
            case PartMask::kLongHair:
            case PartMask::kBody:
            case PartMask::kHands:
            case PartMask::kForearms:
            case PartMask::kFeet:
            case PartMask::kCalves:
            case PartMask::kShield:
            case PartMask::kTail:
            case PartMask::kCloak:
            case PartMask::kBackpack:
                return ArmorWeightClass::kClothing;

            case PartMask::kAmulet:
            case PartMask::kRing:
            case PartMask::kCirclet:
            case PartMask::kEars:
                return ArmorWeightClass::kJewelry;

            default:
                return ArmorWeightClass::kNone;
        }
    }

}  // namespace SkyUI
