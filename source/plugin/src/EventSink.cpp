#include "EventSink.h"
#include "FormCache.h"
#include "TimestampCache.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // InventoryMenuSink — listens for MenuOpenCloseEvent and populates the form
    // cache when the InventoryMenu (or ContainerMenu / BarterMenu, which also
    // show the player's inventory) opens.
    // ---------------------------------------------------------------------------
    class InventoryMenuSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        static InventoryMenuSink* GetSingleton() {
            static InventoryMenuSink instance;
            return &instance;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent*               a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>* /*a_source*/) override
        {
            if (!a_event || !a_event->opening) return RE::BSEventNotifyControl::kContinue;

            // Populate cache whenever a menu that shows the player's inventory opens.
            static constexpr std::string_view kInventoryMenus[] = {
                "InventoryMenu",
                "ContainerMenu",
                "BarterMenu",
                "GiftMenu",
            };

            bool isInventoryMenu = false;
            for (auto name : kInventoryMenus) {
                if (a_event->menuName == name) {
                    isInventoryMenu = true;
                    break;
                }
            }

            if (!isInventoryMenu) return RE::BSEventNotifyControl::kContinue;

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) return RE::BSEventNotifyControl::kContinue;

            FormCache::GetSingleton()->Populate(player);

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        InventoryMenuSink() = default;
    };

    // ---------------------------------------------------------------------------
    // ItemAcquisitionSink — listens for TESContainerChangedEvent and records the
    // acquisition timestamp whenever items move into the player's inventory.
    //
    // TESContainerChangedEvent fires whenever any item moves between containers.
    // We filter for moves where the destination is the player's base form so we
    // only record items the player actually picked up or bought.
    // ---------------------------------------------------------------------------
    class ItemAcquisitionSink : public RE::BSTEventSink<RE::TESContainerChangedEvent> {
    public:
        static ItemAcquisitionSink* GetSingleton() {
            static ItemAcquisitionSink instance;
            return &instance;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESContainerChangedEvent*               a_event,
            RE::BSTEventSource<RE::TESContainerChangedEvent>* /*a_source*/) override
        {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) return RE::BSEventNotifyControl::kContinue;

            // Only care about items moving INTO the player's inventory.
            if (a_event->newContainer != player->GetFormID())
                return RE::BSEventNotifyControl::kContinue;

            if (a_event->baseObj == 0) return RE::BSEventNotifyControl::kContinue;

            TimestampCache::GetSingleton()->Record(a_event->baseObj);

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        ItemAcquisitionSink() = default;
    };

    // ---------------------------------------------------------------------------
    // ---------------------------------------------------------------------------
    void RegisterEventSinks() {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("EventSink: RE::UI singleton not available at kDataLoaded");
            return;
        }
        ui->AddEventSink<RE::MenuOpenCloseEvent>(InventoryMenuSink::GetSingleton());
        logger::info("EventSink: registered InventoryMenuSink");

        auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
        if (!scriptEvents) {
            logger::error("EventSink: ScriptEventSourceHolder not available");
            return;
        }
        scriptEvents->AddEventSink<RE::TESContainerChangedEvent>(ItemAcquisitionSink::GetSingleton());
        logger::info("EventSink: registered ItemAcquisitionSink");
    }

}  // namespace SkyUI
