#pragma once

#include "PCH.h"

namespace SkyUI {

    // Register all SKSE event sinks.  Called from MessageHandler on kDataLoaded.
    void RegisterEventSinks();

}  // namespace SkyUI
