#include "ScaleformAPI.h"
#include "FormCache.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // GetStaticData — ActionScript signature:
    //   SkyUI_SE_GetStaticData(formId: uint) : Object
    //
    // Returns an object with static, form-derived fields for the given formId:
    //   { subType, material, weightClass, mainPartMask, iconLabel, iconColor }
    // Returns undefined on cache miss (item not yet seen in inventory).
    //
    // All integer values match skyui.defines.* constants exactly, so the AS3
    // fast-path can use them directly without re-computing anything.
    // ---------------------------------------------------------------------------
    class GetStaticDataFn : public RE::GFxFunctionHandler {
    public:
        void Call(Params& a_params) override {
            if (a_params.argCount < 1 || !a_params.args[0].IsNumber()) {
                logger::warn("ScaleformAPI: GetStaticData called with no/invalid arg");
                return;  // retVal stays undefined
            }

            const auto formID = static_cast<RE::FormID>(
                static_cast<std::uint32_t>(a_params.args[0].GetNumber()));

            const auto* data = FormCache::GetSingleton()->Get(formID);
            if (!data) {
                // Cache miss — return undefined.  This is expected for items the
                // player has never opened an inventory menu with.
                return;
            }

            // Build the return object.
            a_params.movie->CreateObject(a_params.retVal);

            RE::GFxValue val;

            val.SetNumber(static_cast<double>(data->subType));
            a_params.retVal->SetMember("subType", val);

            val.SetNumber(static_cast<double>(data->material));
            a_params.retVal->SetMember("material", val);

            val.SetNumber(static_cast<double>(data->weightClass));
            a_params.retVal->SetMember("weightClass", val);

            val.SetNumber(static_cast<double>(data->mainPartMask));
            a_params.retVal->SetMember("mainPartMask", val);

            if (!data->iconLabel.empty()) {
                a_params.movie->CreateString(&val, data->iconLabel.c_str());
                a_params.retVal->SetMember("iconLabel", val);
            }

            if (data->iconColor != 0) {
                val.SetNumber(static_cast<double>(data->iconColor));
                a_params.retVal->SetMember("iconColor", val);
            }
        }
    };

    // ---------------------------------------------------------------------------
    // ScaleformRegisterCallback — called by SKSE for every GFx movie loaded.
    // Registers our functions into the movie's root scope.
    // ---------------------------------------------------------------------------
    bool ScaleformRegisterCallback(RE::GFxMovieView* a_view, RE::GFxValue* a_root) {
        RE::GFxValue fn;
        a_view->CreateFunction(&fn, new GetStaticDataFn());
        a_root->SetMember("SkyUI_SE_GetStaticData", fn);

        logger::debug("ScaleformAPI: registered SkyUI_SE_GetStaticData in {}",
                      a_view->GetMovieDef() ? a_view->GetMovieDef()->GetFileURL() : "unknown");

        return true;
    }

}  // namespace SkyUI
