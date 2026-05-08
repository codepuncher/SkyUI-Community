#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>

#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace std::literals;

namespace logger = SKSE::log;
