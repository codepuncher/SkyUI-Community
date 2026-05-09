#pragma once

#include "PCH.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // Integer constants matching skyui.defines.* ActionScript values.
    // These exact values are written into CachedItemData and read by the
    // Scaleform API (Phase 2) so that the AS3 fast-path works without
    // re-computing anything.
    // ---------------------------------------------------------------------------

    // skyui.defines.Weapon TYPE_* constants
    namespace WeaponSubType {
        inline constexpr std::int32_t kMelee      = 0;
        inline constexpr std::int32_t kSword      = 1;
        inline constexpr std::int32_t kDagger     = 2;
        inline constexpr std::int32_t kWarAxe     = 3;
        inline constexpr std::int32_t kMace       = 4;
        inline constexpr std::int32_t kGreatsword = 5;
        inline constexpr std::int32_t kBattleAxe  = 6;
        inline constexpr std::int32_t kWarhammer  = 7;
        inline constexpr std::int32_t kBow        = 8;
        inline constexpr std::int32_t kCrossbow   = 9;
        inline constexpr std::int32_t kStaff      = 10;
        inline constexpr std::int32_t kPickaxe    = 11;
        inline constexpr std::int32_t kWoodAxe    = 12;
        inline constexpr std::int32_t kFishingRod = 13;
        inline constexpr std::int32_t kNone       = -1;
    }

    // skyui.defines.Weapon AMMO_* constants
    namespace AmmoSubType {
        inline constexpr std::int32_t kArrow = 0;
        inline constexpr std::int32_t kBolt  = 1;
    }

    // skyui.defines.Armor EQUIP_* constants
    namespace ArmorSubType {
        inline constexpr std::int32_t kHead     = 0;
        inline constexpr std::int32_t kHair     = 1;
        inline constexpr std::int32_t kLongHair = 2;
        inline constexpr std::int32_t kBody     = 3;
        inline constexpr std::int32_t kForearms = 4;
        inline constexpr std::int32_t kHands    = 5;
        inline constexpr std::int32_t kShield   = 6;
        inline constexpr std::int32_t kCalves   = 7;
        inline constexpr std::int32_t kFeet     = 8;
        inline constexpr std::int32_t kCirclet  = 9;
        inline constexpr std::int32_t kAmulet   = 10;
        inline constexpr std::int32_t kEars     = 11;
        inline constexpr std::int32_t kRing     = 12;
        inline constexpr std::int32_t kTail     = 13;
        inline constexpr std::int32_t kBackpack = 14;
        inline constexpr std::int32_t kCloak    = 15;
        inline constexpr std::int32_t kNone     = -1;
    }

    // skyui.defines.Armor WEIGHT_* constants
    namespace ArmorWeightClass {
        inline constexpr std::int32_t kLight    = 0;
        inline constexpr std::int32_t kHeavy    = 1;
        inline constexpr std::int32_t kNone     = 2;
        inline constexpr std::int32_t kClothing = 3;
        inline constexpr std::int32_t kJewelry  = 4;
        inline constexpr std::int32_t kNA       = -1;
    }

    // skyui.defines.Material.* constants
    namespace MaterialType {
        inline constexpr std::int32_t kAmber      = 0;
        inline constexpr std::int32_t kBonemold   = 1;
        inline constexpr std::int32_t kChitin     = 2;
        inline constexpr std::int32_t kDaedric    = 3;
        inline constexpr std::int32_t kDragon     = 4;
        inline constexpr std::int32_t kDwarven    = 5;
        inline constexpr std::int32_t kEbony      = 6;
        inline constexpr std::int32_t kElven      = 7;
        inline constexpr std::int32_t kFalmer     = 8;
        inline constexpr std::int32_t kGlass      = 9;
        inline constexpr std::int32_t kHide       = 10;
        inline constexpr std::int32_t kImperial   = 11;
        inline constexpr std::int32_t kIron       = 12;
        inline constexpr std::int32_t kLeather    = 13;
        inline constexpr std::int32_t kMadness    = 14;
        inline constexpr std::int32_t kNordic     = 15;
        inline constexpr std::int32_t kOrcish     = 16;
        inline constexpr std::int32_t kOrdinator  = 17;
        inline constexpr std::int32_t kSilver     = 18;
        inline constexpr std::int32_t kStalhrim   = 19;
        inline constexpr std::int32_t kSteel      = 20;
        inline constexpr std::int32_t kStormcloak = 21;
        inline constexpr std::int32_t kWood       = 22;
        inline constexpr std::int32_t kNone       = -1;
    }

    // skyui.defines.Item MISC_* constants
    namespace MiscSubType {
        inline constexpr std::int32_t kGem              = 0;
        inline constexpr std::int32_t kDragonclaw       = 1;
        inline constexpr std::int32_t kArtifact         = 2;
        inline constexpr std::int32_t kLeather          = 3;
        inline constexpr std::int32_t kLeatherStrips    = 4;
        inline constexpr std::int32_t kHide             = 5;
        inline constexpr std::int32_t kRemains          = 6;
        inline constexpr std::int32_t kIngot            = 7;
        inline constexpr std::int32_t kTool             = 8;
        inline constexpr std::int32_t kChildrensClothes = 9;
        inline constexpr std::int32_t kToy              = 10;
        inline constexpr std::int32_t kFirewood         = 11;
        inline constexpr std::int32_t kFastener         = 12;
        inline constexpr std::int32_t kWeaponrack       = 13;
        inline constexpr std::int32_t kShelf            = 14;
        inline constexpr std::int32_t kFurniture        = 15;
        inline constexpr std::int32_t kExterior         = 16;
        inline constexpr std::int32_t kContainer        = 17;
        inline constexpr std::int32_t kHousePart        = 18;
        inline constexpr std::int32_t kClutter          = 19;
        inline constexpr std::int32_t kLockpick         = 20;
        inline constexpr std::int32_t kGold             = 21;
        inline constexpr std::int32_t kTrollskull       = 22;
        inline constexpr std::int32_t kNetchLeather     = 23;
        inline constexpr std::int32_t kAyleidCrystal    = 24;
        inline constexpr std::int32_t kHorseTack        = 25;
        inline constexpr std::int32_t kBrokenWeapon     = 26;
        inline constexpr std::int32_t kDwarvenScrap     = 27;
        inline constexpr std::int32_t kInstrument       = 28;
        inline constexpr std::int32_t kBugJar           = 29;
        inline constexpr std::int32_t kScrollSpider     = 30;
        inline constexpr std::int32_t kOre              = 31;
        inline constexpr std::int32_t kMap              = 32;
        inline constexpr std::int32_t kPotion           = 33;
        inline constexpr std::int32_t kPoison           = 34;
        inline constexpr std::int32_t kScroll           = 35;
        inline constexpr std::int32_t kBook             = 36;
        inline constexpr std::int32_t kRing             = 37;
        inline constexpr std::int32_t kIngredient       = 38;
        inline constexpr std::int32_t kPetGear          = 39;
        inline constexpr std::int32_t kNone             = -1;
    }

    // skyui.defines.Item BOOK_* constants
    namespace BookSubType {
        inline constexpr std::int32_t kSpellTome = 0;
        inline constexpr std::int32_t kNote      = 1;
        inline constexpr std::int32_t kRecipe    = 2;
        inline constexpr std::int32_t kBook      = -1;  // OTHER = undefined → -1 in C++
    }

    // skyui.defines.Item POTION_* constants
    namespace PotionSubType {
        inline constexpr std::int32_t kHealth      = 0;
        inline constexpr std::int32_t kHealRate    = 1;
        inline constexpr std::int32_t kMagicka     = 3;
        inline constexpr std::int32_t kMagickaRate = 4;
        inline constexpr std::int32_t kStamina     = 6;
        inline constexpr std::int32_t kStaminaRate = 7;
        inline constexpr std::int32_t kPotion      = 12;
        inline constexpr std::int32_t kDrink       = 13;
        inline constexpr std::int32_t kFood        = 14;
        inline constexpr std::int32_t kPoison      = 15;
    }

    // ---------------------------------------------------------------------------
    // CachedItemData — the static, form-derived fields that never change at
    // runtime.  Populated once when the inventory menu opens for the first time
    // with a given form, then served from cache on every subsequent open.
    // ---------------------------------------------------------------------------
    struct CachedItemData {
        std::int32_t  subType{ -1 };       // weapon/armor/book/potion type constant
        std::int32_t  material{ -1 };      // MaterialType::k* constant, -1 = none
        std::int32_t  weightClass{ -1 };   // ArmorWeightClass::k*, -1 = not armor
        std::uint32_t mainPartMask{ 0 };   // dominant biped slot bitmask (armor only)
    };

    // ---------------------------------------------------------------------------
    // FormCache — singleton cache keyed by FormID.
    // Populate() is called from the MenuOpenCloseEvent sink on InventoryMenu open.
    // Get() is called from the Scaleform API (Phase 2) per-item during list build.
    // ---------------------------------------------------------------------------
    class FormCache {
    public:
        [[nodiscard]] static FormCache* GetSingleton();

        // Warm up static lookup tables on the main thread at kDataLoaded, before
        // any concurrent Populate() calls can race on the first initialization.
        static void Initialize();

        // Walk a_actor's inventory and insert any unseen forms into the cache.
        void Populate(RE::Actor* a_actor);

        // Evict all entries (called on kNewGame / kPostLoadGame).
        void Clear();

        // Look up cached data for a formID.  Returns nullptr on cache miss.
        [[nodiscard]] const CachedItemData* Get(RE::FormID a_formID) const;

    private:
        FormCache() = default;

        mutable std::shared_mutex                       _mutex;
        std::unordered_map<RE::FormID, CachedItemData>  _cache;

        static CachedItemData BuildItemData(RE::TESBoundObject* a_item);
        static CachedItemData BuildWeaponData(RE::TESObjectWEAP* a_weapon);
        static CachedItemData BuildArmorData(RE::TESObjectARMO* a_armor);
        static CachedItemData BuildAmmoData(RE::TESAmmo* a_ammo);
        static CachedItemData BuildBookData(RE::TESObjectBOOK* a_book);
        static CachedItemData BuildAlchemyData(RE::AlchemyItem* a_alchemy);
        static CachedItemData BuildIngredientData(RE::IngredientItem* a_ingredient);
        static CachedItemData BuildKeyData(RE::TESKey* a_key);
        static CachedItemData BuildSoulGemData(RE::TESSoulGem* a_soulGem);
        static CachedItemData BuildScrollData(RE::ScrollItem* a_scroll);
        static CachedItemData BuildMiscData(RE::TESObjectMISC* a_misc);

        // Keyword-based material lookup, mirrors processMaterialKeywords() in AS3.
        static std::int32_t GetMaterial(RE::BGSKeywordForm* a_kwForm);

        // Walk the biped part-mask precedence list (mirrors PARTMASK_PRECEDENCE
        // in Armor.as) to find the highest-priority slot this piece occupies.
        static std::uint32_t GetMainPartMask(std::uint32_t a_partMask);

        // Map mainPartMask → EQUIP_* subType constant (mirrors processArmorPartMask).
        static std::int32_t ArmorSubTypeFromPartMask(std::uint32_t a_mainPartMask);

        // Resolve final weightClass for pieces that report WEIGHT_NONE from the
        // engine (clothing/jewelry disambiguation, mirrors processArmorOther).
        static std::int32_t ResolveWeightClass(std::int32_t a_engineWeightClass,
                                               std::uint32_t a_mainPartMask,
                                               RE::BGSKeywordForm* a_kwForm);
    };

}  // namespace SkyUI
