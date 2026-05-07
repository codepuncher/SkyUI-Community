#include "PCH.h"

namespace {
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kDataLoaded:
                logger::info("SkyUI plugin: data loaded");
                break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    auto path = logger::log_directory();
    if (!path) return false;
    *path /= "SkyUI_SE.log";
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log  = std::make_shared<spdlog::logger>("SkyUI_SE", std::move(sink));
    log->set_level(spdlog::level::debug);
    log->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(std::move(log));

    auto* messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    logger::info("SkyUI_SE plugin v{} loaded", SKSE::PluginDeclaration::GetSingleton()->GetVersion().string());

    return true;
}
