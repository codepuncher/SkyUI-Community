#include "TimestampCache.h"

namespace SkyUI {

    TimestampCache* TimestampCache::GetSingleton() {
        static TimestampCache instance;
        return &instance;
    }

    void TimestampCache::Record(RE::FormID a_formID) {
        const auto now = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
            .count());

        std::unique_lock lock(_mutex);
        _cache[a_formID] = now;
    }

    std::uint64_t TimestampCache::Get(RE::FormID a_formID) const {
        std::shared_lock lock(_mutex);
        const auto it = _cache.find(a_formID);
        return it != _cache.end() ? it->second : 0ULL;
    }

    void TimestampCache::Clear() {
        std::unique_lock lock(_mutex);
        _cache.clear();
    }

    // ---------------------------------------------------------------------------
    // Serialization — persist timestamps across game saves.
    //
    // Format (per record):
    //   uint32_t formID
    //   uint64_t timestamp
    // ---------------------------------------------------------------------------

    void TimestampCache::Save(SKSE::SerializationInterface* a_intfc) const {
        std::shared_lock lock(_mutex);

        if (!a_intfc->OpenRecord(kSerializationKey, kSerializationVersion)) {
            logger::error("TimestampCache: failed to open serialization record");
            return;
        }

        const auto count = static_cast<std::uint32_t>(_cache.size());
        a_intfc->WriteRecordData(count);

        for (const auto& [formID, timestamp] : _cache) {
            a_intfc->WriteRecordData(formID);
            a_intfc->WriteRecordData(timestamp);
        }

        logger::debug("TimestampCache: saved {} entries", count);
    }

    void TimestampCache::Load(SKSE::SerializationInterface* a_intfc) {
        std::unique_lock lock(_mutex);
        _cache.clear();

        std::uint32_t count = 0;
        if (!a_intfc->ReadRecordData(count)) {
            logger::error("TimestampCache: failed to read entry count");
            return;
        }

        std::uint32_t loaded = 0;
        for (std::uint32_t i = 0; i < count; ++i) {
            RE::FormID    formID    = 0;
            std::uint64_t timestamp = 0;

            if (!a_intfc->ReadRecordData(formID) || !a_intfc->ReadRecordData(timestamp)) {
                logger::error("TimestampCache: truncated record at entry {}", i);
                break;
            }

            // Resolve the FormID through SKSE's form-version table — required
            // after a load because mod list changes can remap FormIDs.
            RE::FormID resolvedID = formID;
            if (!a_intfc->ResolveFormID(formID, resolvedID)) {
                logger::debug("TimestampCache: skipping unresolvable FormID {:08X}", formID);
                continue;
            }

            _cache[resolvedID] = timestamp;
            ++loaded;
        }

        logger::debug("TimestampCache: loaded {} entries ({} skipped)", loaded, count - loaded);
    }

}  // namespace SkyUI
