#pragma once

#include "PCH.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // TimestampCache — records the real-world Unix timestamp (seconds since epoch)
    // of the most recent acquisition of each base-form item by the player.
    //
    // "Last acquisition wins": picking up any quantity of an item (including
    // adding to an existing stack) always overwrites the stored timestamp.
    //
    // Keyed by base FormID — stackable items share one timestamp since the engine
    // provides no per-instance identity for them.
    //
    // Thread-safe via shared_mutex: Get() takes a shared lock; Record()/Clear()
    // take exclusive locks.
    // ---------------------------------------------------------------------------
    class TimestampCache {
    public:
        [[nodiscard]] static TimestampCache* GetSingleton();

        // Record the current real-world time for a_formID (always overwrites).
        void Record(RE::FormID a_formID);

        // Return the stored Unix timestamp for a_formID, or 0 if unknown.
        [[nodiscard]] std::uint64_t Get(RE::FormID a_formID) const;

        // Evict all entries (called on kNewGame / Revert serialization callback).
        void Clear();

        // ---------------------------------------------------------------------------
        // SKSE co-save serialization
        // ---------------------------------------------------------------------------
        static constexpr std::uint32_t kSerializationKey     = 'SKTS';
        static constexpr std::uint32_t kSerializationVersion = 1;

        // Called by SKSE serialization Save callback.
        void Save(SKSE::SerializationInterface* a_intfc) const;

        // Called by SKSE serialization Load callback.
        void Load(SKSE::SerializationInterface* a_intfc);

    private:
        TimestampCache() = default;

        mutable std::shared_mutex                          _mutex;
        std::unordered_map<RE::FormID, std::uint64_t>     _cache;
    };

}  // namespace SkyUI
