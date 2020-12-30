// Fallback copy of the key bindings file we can load in the case that the
// key bindings file is not present and then we just use these defaults.

static constexpr const char* gKeyBindingsFallback =
"level_new { main [\"Ctrl\" \"N\"] }\n"
"level_open { main [\"Ctrl\" \"O\"] }\n"
"level_save  { main [\"Ctrl\" \"S\"] }\n"
"level_save_as { main [\"Ctrl\" \"Shift\" \"S\"] }\n"
"level_close { main [\"Ctrl\" \"W\"] }\n"
"level_close_all { main [\"Ctrl\" \"Alt\" \"W\"] }\n"
"level_resize { main [\"Ctrl\" \"R\"] }\n"
"undo { main [\"Ctrl\" \"Z\"] }\n"
"redo { main [\"Ctrl\" \"Y\"] }\n"
"history_begin { main [\"Ctrl\" \"Shift\" \"Z\"] }\n"
"history_end { main [\"Ctrl\" \"Shift\" \"Y\"] }\n"
"camera_zoom_out { main [\"Ctrl\" \"-\"] }\n"
"camera_zoom_in { main [\"Ctrl\" \"=\"] }\n"
"run_game { main [\"F5\"] }\n"
"preferences { main [\"F2\"] }\n"
"about { main [\"F3\"] }\n"
"bug_report { main [\"F4\"] }\n"
"help { main [\"F1\"] }\n"
"tool_brush { main [\"B\"] }\n"
"tool_fill { main [\"F\"] }\n"
"tool_select { main [\"S\"] }\n"
"flip_h { main [\"J\"] }\n"
"flip_v { main [\"K\"] }\n"
"mirror_h_toggle { main [\"N\"] }\n"
"mirror_v_toggle { main [\"M\"] }\n"
"gpak_unpack { main [\"Ctrl\" \"U\"] }\n"
"gpak_pack { main [\"Ctrl\" \"P\"] }\n"
"toggle_large_entity { main [\"Ctrl\" \"L\"] }\n"
"toggle_entity_guides { main [\"Ctrl\" \"G\"] }\n"
"grid_toggle { main [\"G\"] }\n"
"bounds_toggle { main [\"C\"] }\n"
"layer_trans_toggle { main [\"L\"] }\n"
"camera_reset { main [\"Ctrl\" \"0\"] }\n"
"clear_select { main [\"Delete\"] }\n"
"deselect { main [\"Ctrl\" \"D\"] }\n"
"select_all { main [\"Ctrl\" \"A\"] }\n"
"copy { main [\"Ctrl\" \"C\"] }\n"
"cut { main [\"Ctrl\" \"X\"] }\n"
"paste { main [\"Ctrl\" \"V\"] }\n"
"increment_tile { main [\"E\"] }\n"
"decrement_tile { main [\"Q\"] }\n"
"increment_group { main [\"Alt\" \"E\"] }\n"
"decrement_group { main [\"Alt\" \"Q\"] }\n"
"increment_category { main [\"Alt\" \"Shift\" \"E\"] }\n"
"decrement_category { main [\"Alt\" \"Shift\" \"Q\"] }\n"
"toggle_layer_active { main [\"Ctrl\" \"1\"] alt [\"Ctrl\" \"4\"] }\n"
"toggle_layer_tag { main [\"Ctrl\" \"2\"] }\n"
"toggle_layer_overlay { main [\"Ctrl\" \"3\"] }\n"
"toggle_layer_back1 { main [\"Ctrl\" \"5\"] }\n"
"toggle_layer_back2 { main [\"Ctrl\" \"6\"] }\n"
"category_basic { main [\"1\"] }\n"
"category_tag { main [\"2\"] }\n"
"category_overlay { main [\"3\"] }\n"
"category_entity { main [\"4\"] }\n"
"category_back1 { main [\"5\"] }\n"
"category_back2 { main [\"6\"] }\n"
"increment_tab { main [\"Ctrl\" \"Tab\"] }\n"
"decrement_tab { main [\"Ctrl\" \"Shift\" \"Tab\"] }\n"
"move_tab_left { main [\"Left\"] }\n"
"move_tab_right { main [\"Right\"] }\n"
"open_recent_tab { main [\"Ctrl\" \"Shift\" \"T\"] }\n"
"load_prev_level { main [\"Ctrl\" \"Left\"] }\n"
"load_next_level { main [\"Ctrl\" \"Right\"] }\n";

typedef std::pair<std::string,KeyBinding> KBPair;

static const std::map<std::string,int> gKeyModMap
{
{ "None",  0          },
{ "Ctrl",  KMOD_CTRL  },
{ "Alt",   KMOD_ALT   },
{ "AltGr", KMOD_MODE  },
{ "Shift", KMOD_SHIFT },
{ "Gui",   KMOD_GUI   }
};

namespace Internal
{
    TEINAPI bool KeyBindingActive (const KeyBinding& kb)
    {
        // Not all key bindings necessarily have a key code so we assign a
        // default value of true. However, key mods should always be checked
        // even if they are not present. Otherwise issues can occur where
        // key bindings get checked as active even when they shouldn't be.
        bool codeActive = true, altCodeActive = true;
        bool modActive = false, altModActive = false;

        if (kb.code) codeActive = IsKeyCodeActive(kb.code);
        modActive = IsKeyModStateActive(kb.mod);

        if (kb.hasAlt)
        {
            if (kb.altCode) altCodeActive = IsKeyCodeActive(kb.altCode);
            altModActive = IsKeyModStateActive(kb.altMod);
        }

        return (codeActive && modActive) || (kb.hasAlt && (altCodeActive && altModActive));
    }

    TEINAPI std::string GetKeyBindingString (int code, int mod)
    {
        std::string str;
        if (mod)
        {
            if (mod & KMOD_CTRL ) str +=  "Ctrl ";
            if (mod & KMOD_ALT  ) str +=   "Alt ";
            if (mod & KMOD_MODE ) str += "AltGr ";
            if (mod & KMOD_SHIFT) str += "Shift ";
            if (mod & KMOD_GUI  ) str +=   "Gui ";
        }

        str += SDL_GetKeyName(code);

        // Trim trailing whitespace from the key binding name.
        size_t pos = str.find_last_not_of(" ");
        if (pos != std::string::npos) str.erase(pos+1);

        return str;
    }

    TEINAPI void AddKeyBinding (const GonObject& gon, const GonObject& gonFallback, const char* name, KBAction action)
    {
        // Assign both to zero in case there are no values.
        int code = 0, altCode = 0;
        int mod = 0, altMod = 0;

        bool gonContains = gon.Contains(name);
        if (!gonContains && !gonFallback.Contains(name))
        {
            LogError(ERR_MIN, "No key binding for '%s'!", name);
            return;
        }

        if (gKeyBindings.count(name))
        {
            LogError(ERR_MED, "Duplicate key binding '%s'!", name);
            return;
        }

        const GonObject& keyBinding = (gonContains) ? gon[name] : gonFallback[name];
        if (keyBinding.Contains("main"))
        {
            const GonObject& kbMain = keyBinding["main"];
            for (int i=0; i<kbMain.size(); ++i)
            {
                // Convert the key mod and code names into actual values.
                std::string keyName = kbMain[i].String();
                SDL_Keycode keyCode = SDL_GetKeyFromName(keyName.c_str());

                if (keyCode != SDLK_UNKNOWN)
                {
                    code = keyCode;
                }
                else
                {
                    if (gKeyModMap.count(keyName))
                    {
                        mod |= gKeyModMap.at(keyName);
                    }
                    else
                    {
                        LogError(ERR_MED, "Invalid key mod '%s'!", keyName.c_str());
                        return;
                    }
                }
            }
        }

        bool hasAlt = keyBinding.Contains("alt");
        if (hasAlt)
        {
            const GonObject& kbAlt = keyBinding["alt"];
            for (int i=0; i<kbAlt.size(); ++i)
            {
                // Convert the key mod and code names into actual values.
                std::string keyName = kbAlt[i].String();
                SDL_Keycode keyCode = SDL_GetKeyFromName(keyName.c_str());

                if (keyCode != SDLK_UNKNOWN)
                {
                    altCode = keyCode;
                }
                else
                {
                    if (gKeyModMap.count(keyName))
                    {
                        altMod |= gKeyModMap.at(keyName);
                    }
                    else
                    {
                        LogError(ERR_MED, "Invalid key mod '%s'!", keyName.c_str());
                        return;
                    }
                }
            }
        }

        // If neither are set then input was malformed or empty.
        if (!code && !mod)
        {
            LogError(ERR_MED, "Invalid key binding '%s'!", name);
            return;
        }

        KeyBinding kb = {};
        kb.action = action;
        kb.hasAlt = hasAlt;
        kb.mod = mod;
        kb.code = code;
        kb.altMod = altMod;
        kb.altCode = altCode;

        gKeyBindings.insert({ name, kb });
    }

    TEINAPI void LoadEditorKeyBindings (const GonObject& a, const GonObject& b)
    {
        gKeyBindings.clear();

        AddKeyBinding(a, b, gKbLevelNew,           HotbarNew);
        AddKeyBinding(a, b, gKbLevelOpen,          HotbarLoad);
        AddKeyBinding(a, b, gKbLevelSave,          HotbarSave);
        AddKeyBinding(a, b, gKbLevelSaveAs,        HotbarSaveAs);
        AddKeyBinding(a, b, gKbLevelClose,         close_current_tab);
        AddKeyBinding(a, b, gKbLevelCloseAll,      close_all_tabs);
        AddKeyBinding(a, b, gKbLevelResize,        LevelEditorResize);
        AddKeyBinding(a, b, gKbUndo,               HotbarUndo);
        AddKeyBinding(a, b, gKbRedo,               HotbarRedo);
        AddKeyBinding(a, b, gKbHistoryBegin,       HotbarHistoryBegin);
        AddKeyBinding(a, b, gKbHistoryEnd,         HotbarHistoryEnd);
        AddKeyBinding(a, b, gKbCameraZoomOut,      HotbarZoomOut);
        AddKeyBinding(a, b, gKbCameraZoomIn,       HotbarZoomIn);
        AddKeyBinding(a, b, gKbRunGame,            HotbarRunGame);
        AddKeyBinding(a, b, gKbPreferences,        HotbarPreferences);
        AddKeyBinding(a, b, gKbAbout,              HotbarAbout);
        AddKeyBinding(a, b, gKbBugReport,          HotbarBugReport);
        AddKeyBinding(a, b, gKbHelp,               HotbarHelp);
        AddKeyBinding(a, b, gKbToolBrush,          ToolbarSetToolToBrush);
        AddKeyBinding(a, b, gKbToolFill,           ToolbarSetToolToFill);
        AddKeyBinding(a, b, gKbToolSelect,         ToolbarSetToolToSelect);
        AddKeyBinding(a, b, gKbFlipH,              ToolbarFlipLevelH);
        AddKeyBinding(a, b, gKbFlipV,              ToolbarFlipLevelV);
        AddKeyBinding(a, b, gKbMirrorHToggle,      ToolbarToggleMirrorH);
        AddKeyBinding(a, b, gKbMirrorVToggle,      ToolbarToggleMirrorV);
        AddKeyBinding(a, b, gKbGpakUnpack,         HotbarGPAKUnpack);
        AddKeyBinding(a, b, gKbGpakPack,           HotbarGPAKPack);
        AddKeyBinding(a, b, gKbToggleEntity,       ToolbarToggleEntity);
        AddKeyBinding(a, b, gKbToggleGuides,       ToolbarToggleGuides);
        AddKeyBinding(a, b, gKbGridToggle,         ToolbarToggleGrid);
        AddKeyBinding(a, b, gKbBoundsToggle,       ToolbarToggleGuides);
        AddKeyBinding(a, b, gKbLayersToggle,       ToolbarToggleLayerTransparency);
        AddKeyBinding(a, b, gKbCameraReset,        ToolbarResetCamera);
        AddKeyBinding(a, b, gKbClearSelect,        ToolbarClearSelect);
        AddKeyBinding(a, b, gKbDeselect,           ToolbarDeselect);
        AddKeyBinding(a, b, gKbSelectAll,          editor_select_all);
        AddKeyBinding(a, b, gKbCopy,               ToolbarCopy);
        AddKeyBinding(a, b, gKbCut,                ToolbarCut);
        AddKeyBinding(a, b, gKbPaste,              editor_paste);
        AddKeyBinding(a, b, gKbIncrementTile,      IncrementSelectedTile);
        AddKeyBinding(a, b, gKbDecrementTile,      DecrementSelectedTile);
        AddKeyBinding(a, b, gKbIncrementGroup,     IncrementSelectedGroup);
        AddKeyBinding(a, b, gKbDecrementGroup,     DecrementSelectedGroup);
        AddKeyBinding(a, b, gKbIncrementCategory,  IncrementSelectedCategory);
        AddKeyBinding(a, b, gKbDecrementCategory,  DecrementSelectedCategory);
        AddKeyBinding(a, b, gKbToggleLayerActive,  ToggleLayerActive);
        AddKeyBinding(a, b, gKbToggleLayerTag,     ToggleLayerTag);
        AddKeyBinding(a, b, gKbToggleLayerOverlay, ToggleLayerOverlay);
        AddKeyBinding(a, b, gKbToggleLayerBack1,   ToggleLayerBack1);
        AddKeyBinding(a, b, gKbToggleLayerBack2,   ToggleLayerBack2);
        AddKeyBinding(a, b, gKbCategoryBasic,      JumpToCategoryBasic);
        AddKeyBinding(a, b, gKbCategoryTag,        JumpToCategoryTag);
        AddKeyBinding(a, b, gKbCategoryOverlay,    JumpToCategoryOverlay);
        AddKeyBinding(a, b, gKbCategoryEntity,     JumpToCategoryEntity);
        AddKeyBinding(a, b, gKbCategoryBack1,      JumpToCategoryBack1);
        AddKeyBinding(a, b, gKbCategoryBack2,      JumpToCategoryBack2);
        AddKeyBinding(a, b, gKbIncrementTab,       increment_tab);
        AddKeyBinding(a, b, gKbDecrementTab,       decrement_tab);
        AddKeyBinding(a, b, gKbMoveTabLeft,        MoveTabLeft);
        AddKeyBinding(a, b, gKbMoveTabRight,       MoveTabRight);
        AddKeyBinding(a, b, gKbOpenRecentTab,      open_recently_closed_tab);
        AddKeyBinding(a, b, gKbLoadPrevLevel,      LevelEditorLoadPrevLevel);
        AddKeyBinding(a, b, gKbLoadNextLevel,      LevelEditorLoadNextLevel);
    }
}

TEINAPI inline bool operator== (const KeyBinding& a, const KeyBinding& b)
{
    return ((a.action == b.action) && (a.code == b.code) && (a.mod == b.mod) && (a.altCode == b.altCode) && (a.altMod == b.altMod));
}
TEINAPI inline bool operator!= (const KeyBinding& a, const KeyBinding& b)
{
    return !(a == b);
}

TEINAPI bool LoadEditorKeyBindings ()
{
    GonObject gon;
    try
    {
        std::string fileName(MakePathAbsolute(gKeyBindingsFileName));
        gon = GonObject::Load(fileName);
    }
    catch (const char* msg)
    {
        LogError(ERR_MED, "%s", msg);

        // If we already have key bind data then we just inform the user that the operation
        // failed. Otherwise, we just fallback to using the default application key binds.
        if (!gKeyBindings.empty())
        {
            LogError(ERR_MED, "Failed to reload key bindings data!");
            return false;
        }
        else
        {
            gon = GonObject::LoadFromBuffer(gKeyBindingsFallback);
        }
    }

    // If we reach this point and there are no key binds then we just use the defaults.
    // This could be the case if the key binds failed to load or haven't been modified.
    if (gon.type != GonObject::FieldType::OBJECT)
    {
        gon = GonObject::LoadFromBuffer(gKeyBindingsFallback);
    }

    GonObject gonFallback = GonObject::LoadFromBuffer(gKeyBindingsFallback);

    Internal::LoadEditorKeyBindings(gon, gonFallback);
    LogDebug("Loaded Editor Key Bindings");

    return true;
}

TEINAPI void RestoreEditorKeyBindings ()
{
    // Load the fallback/default editor key bindings so they can be restored.
    GonObject gon = GonObject::LoadFromBuffer(gKeyBindingsFallback);
    Internal::LoadEditorKeyBindings(gon, gon);
}

TEINAPI void HandleKeyBindingEvents ()
{
    if (!TextBoxIsActive() || IsWindowFocused("WINMAIN"))
    {
        // We only care about key press events, anything else is ignored.
        if (main_event.type == SDL_KEYDOWN)
        {
            for (auto [name,kb]: gKeyBindings)
            {
                // Check that the appropriate keys and mods are pressed.
                if (Internal::KeyBindingActive(kb))
                {
                    // Do not attempt to call a NULL value!
                    if (kb.action) kb.action();
                }
            }
        }
    }
}

TEINAPI const KeyBinding& GetKeyBinding (std::string name)
{
    return gKeyBindings[name];
}

TEINAPI std::string GetKeyBindingMainString (const KeyBinding& kb)
{
    return Internal::GetKeyBindingString(kb.code, kb.mod);
}
TEINAPI std::string GetKeyBindingMainString (std::string name)
{
    return GetKeyBindingMainString(GetKeyBinding(name));
}

TEINAPI std::string GetKeyBindingAltString (const KeyBinding& kb)
{
    // There is no alternative key binding for this particular key.
    if (!kb.altCode && !kb.altMod) return std::string();
    return Internal::GetKeyBindingString(kb.altCode, kb.altMod);
}
TEINAPI std::string GetKeyBindingAltString (std::string name)
{
    return GetKeyBindingAltString(GetKeyBinding(name));
}

TEINAPI bool IsKeyBindingActive (std::string name)
{
    if (TextBoxIsActive() || !IsWindowFocused("WINMAIN")) return false;

    // If the key binding doesn't exist then it can't be active.
    if (!gKeyBindings.count(name))
    {
        LogError(ERR_MIN, "No key binding with name '%s'!", name.c_str());
        return false;
    }

    return Internal::KeyBindingActive(gKeyBindings.at(name));
}

TEINAPI bool IsKeyModStateActive (int mod)
{
    if (TextBoxIsActive() || !IsWindowFocused("WINMAIN")) return false;

    // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
    int currentMod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));
    // We do not care whether the right or left variants have been pressed.
    if ((currentMod & KMOD_LCTRL ) || (currentMod & KMOD_RCTRL )) currentMod |= KMOD_CTRL;
    if ((currentMod & KMOD_LALT  ) || (currentMod & KMOD_RALT  )) currentMod |= KMOD_ALT;
    if ((currentMod & KMOD_LSHIFT) || (currentMod & KMOD_RSHIFT)) currentMod |= KMOD_SHIFT;
    if ((currentMod & KMOD_LGUI  ) || (currentMod & KMOD_RGUI  )) currentMod |= KMOD_GUI;

    return (currentMod == mod);
}

TEINAPI bool IsKeyCodeActive (int code)
{
    int keyCount; // So we know the length of the returned array.
    const U8* keyState = SDL_GetKeyboardState(&keyCount);
    // Need to convert the keycode to a scancode for this.
    SDL_Scancode scancode = SDL_GetScancodeFromKey(code);
    return (scancode < keyCount) ? keyState[scancode] : false;
}
