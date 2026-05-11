#include "PCH.h"
#include "EventSink.h"
#include "FormCache.h"
#include "Plugin.h"
#include "ScaleformAPI.h"
#include "TimestampCache.h"

namespace {
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kDataLoaded:
                logger::info("SkyUI plugin: data loaded");
                SkyUI::FormCache::GetSingleton()->Initialize();
                SkyUI::RegisterEventSinks();
                break;
            case SKSE::MessagingInterface::kNewGame:
                // On a new game there is no cosave, so nothing was loaded into
                // TimestampCache. Clear both caches so there is no stale data.
                SkyUI::FormCache::GetSingleton()->Clear();
                SkyUI::TimestampCache::GetSingleton()->Clear();
                break;
            case SKSE::MessagingInterface::kPostLoadGame:
                // TimestampCache is already populated by LoadCallback (SKSE
                // serialization) before this message fires — do NOT clear it.
                // FormCache is in-memory only and must be rebuilt on next open.
                SkyUI::FormCache::GetSingleton()->Clear();
                break;
        }
    }

    void SaveCallback(SKSE::SerializationInterface* a_intfc) {
        SkyUI::TimestampCache::GetSingleton()->Save(a_intfc);
    }

    void LoadCallback(SKSE::SerializationInterface* a_intfc) {
        std::uint32_t type, version, length;
        while (a_intfc->GetNextRecordInfo(type, version, length)) {
            if (type == SkyUI::TimestampCache::kSerializationKey) {
                SkyUI::TimestampCache::GetSingleton()->Load(a_intfc);
            } else {
                logger::warn("TimestampCache: unknown record type {:08X} — skipping", type);
            }
        }
    }

    void RevertCallback(SKSE::SerializationInterface*) {
        SkyUI::TimestampCache::GetSingleton()->Clear();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    auto path = logger::log_directory();
    if (!path) return false;
    *path /= (std::string(SkyUI::Plugin::kName) + ".log");
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log  = std::make_shared<spdlog::logger>(SkyUI::Plugin::kName, std::move(sink));
    log->set_level(spdlog::level::debug);
    log->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(std::move(log));

    auto* messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(SkyUI::TimestampCache::kSerializationKey);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    serialization->SetRevertCallback(RevertCallback);

    auto* scaleform = SKSE::GetScaleformInterface();
    scaleform->Register(SkyUI::ScaleformRegisterCallback, SkyUI::Plugin::kName);

    logger::info("{} plugin v{} loaded", SkyUI::Plugin::kName, SKSE::PluginDeclaration::GetSingleton()->GetVersion().string());

    return true;
}
