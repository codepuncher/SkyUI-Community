#include "ScaleformAPI.h"
#include "FormCache.h"
#include "Plugin.h"
#include "TimestampCache.h"

namespace SkyUI {

    // ---------------------------------------------------------------------------
    // GetStaticData — ActionScript signature:
    //   SkyUI_SE_GetStaticData(formId: uint) : Object
    //
    // Returns an object with static, form-derived fields for the given formId:
    //   { subType, material, weightClass, mainPartMask }
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

            if (data->subTypeDisplayKey) {
                val.SetString(data->subTypeDisplayKey);
                a_params.retVal->SetMember("subTypeDisplay", val);
            }

            // Acquired timestamp — 0 if never recorded (item predates the feature).
            const auto ts = TimestampCache::GetSingleton()->Get(formID);
            val.SetNumber(static_cast<double>(ts));
            a_params.retVal->SetMember("acquiredTimestamp", val);
        }
    };

    // ---------------------------------------------------------------------------
    // Log — ActionScript signature:
    //   SkyUI_SE_Log(msg: String) : Void
    //
    // Writes a message from AS2 directly into the SKSE plugin log.
    // Used by the benchmark timing code in InventoryDataSetter.processList.
    // ---------------------------------------------------------------------------
    class LogFn : public RE::GFxFunctionHandler {
    public:
        void Call(Params& a_params) override {
            if (a_params.argCount < 1) return;
            const auto str = a_params.args[0].ToString();
            logger::info("[AS2] {}", str.c_str());
        }
    };

    // ---------------------------------------------------------------------------
    // ScaleformRegisterCallback — called by SKSE for every GFx movie loaded.
    // Registers our functions into both _root and _global.
    //
    // _root is a MovieClip property — accessible from timeline frame scripts but
    // NOT from plain AS2 class instances (e.g. InventoryDataSetter, which extends
    // ItemcardDataExtender, not MovieClip).  _global is accessible from any scope
    // including class methods, so we register on both for maximum compatibility.
    // ---------------------------------------------------------------------------
    bool ScaleformRegisterCallback(RE::GFxMovieView* a_view, RE::GFxValue* a_root) {
        RE::GFxValue fnGetStaticData;
        a_view->CreateFunction(&fnGetStaticData, new GetStaticDataFn());

        RE::GFxValue fnLog;
        a_view->CreateFunction(&fnLog, new LogFn());

        // Register on _root (for any timeline/MovieClip code)
        a_root->SetMember(Plugin::kFnGetStaticData, fnGetStaticData);
        a_root->SetMember(Plugin::kFnLog, fnLog);

        // Also register on _global so plain class instances can reach them
        RE::GFxValue globalObj;
        a_view->GetVariable(&globalObj, "_global");
        if (globalObj.IsObject()) {
            globalObj.SetMember(Plugin::kFnGetStaticData, fnGetStaticData);
            globalObj.SetMember(Plugin::kFnLog, fnLog);
        }

        logger::debug("ScaleformAPI: registered {} in {}",
                      Plugin::kFnGetStaticData,
                      a_view->GetMovieDef() ? a_view->GetMovieDef()->GetFileURL() : "unknown");

        return true;
    }

}  // namespace SkyUI
