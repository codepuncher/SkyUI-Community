#pragma once

#include "PCH.h"

namespace SkyUI {

    // RegCallback registered with SKSE::GetScaleformInterface()->Register().
    // Called by SKSE for every GFx movie loaded; registers our C++ functions
    // into the movie's root scope so ActionScript can call them.
    bool ScaleformRegisterCallback(RE::GFxMovieView* a_view, RE::GFxValue* a_root);

}  // namespace SkyUI
