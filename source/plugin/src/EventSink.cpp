#include "EventSink.h"
#include "FormCache.h"

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
    // RegisterEventSinks — called once from MessageHandler on kDataLoaded.
    // ---------------------------------------------------------------------------
    void RegisterEventSinks() {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("EventSink: RE::UI singleton not available at kDataLoaded");
            return;
        }
        ui->AddEventSink<RE::MenuOpenCloseEvent>(InventoryMenuSink::GetSingleton());
        logger::info("EventSink: registered InventoryMenuSink");
    }

}  // namespace SkyUI
