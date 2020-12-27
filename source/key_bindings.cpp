/*******************************************************************************
 * System for binding specific keys to actions and handling them when pressed.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// Fallback copy of the key bindings file we can load in the case that the
// key bindings file is not present and then we just use these defaults.

static constexpr const char* KEY_BINDINGS_FALLBACK =
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

/* -------------------------------------------------------------------------- */

typedef std::pair<std::string, Key_Binding> KB_Pair;

static const std::map<std::string, int> KEY_MOD_MAP
{
{ "None",  0          },
{ "Ctrl",  KMOD_CTRL  },
{ "Alt",   KMOD_ALT   },
{ "AltGr", KMOD_MODE  },
{ "Shift", KMOD_SHIFT },
{ "Gui",   KMOD_GUI   }
};

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__key_binding_active (const Key_Binding& kb)
{
    // Not all key bindings necessarily have a key code so we assign a
    // default value of true. However, key mods should always be checked
    // even if they are not present. Otherwise issues can occur where
    // key bindings get checked as active even when they shouldn't be.
    bool code_active = true, alt_code_active = true;
    bool mod_active = false, alt_mod_active = false;

    if (kb.code) code_active = is_key_code_active(kb.code);
    mod_active = is_key_mod_state_active(kb.mod);

    if (kb.has_alt)
    {
        if (kb.alt_code) alt_code_active = is_key_code_active(kb.alt_code);
        alt_mod_active = is_key_mod_state_active(kb.alt_mod);
    }

    return (code_active && mod_active) || (kb.has_alt && (alt_code_active && alt_mod_active));
}

TEINAPI std::string internal__get_key_binding_string (int code, int mod)
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

TEINAPI void internal__add_key_binding (const GonObject& gon, const GonObject& gon_fallback, const char* name, KB_Action action)
{
    // Assign both to zero in case there are no values.
    int code = 0, alt_code = 0;
    int mod = 0, alt_mod = 0;

    bool gon_contains = gon.Contains(name);
    if (!gon_contains && !gon_fallback.Contains(name))
    {
        LOG_ERROR(ERR_MIN, "No key binding for '%s'!", name);
        return;
    }

    if (key_bindings.count(name) > 0)
    {
        LOG_ERROR(ERR_MED, "Duplicate key binding '%s'!", name);
        return;
    }

    const GonObject& key_binding = (gon_contains) ? gon[name] : gon_fallback[name];
    if (key_binding.Contains("main"))
    {
        const GonObject& kb_main = key_binding["main"];
        for (int i=0; i<kb_main.size(); ++i)
        {
            // Convert the key mod and code names into actual values.
            std::string key_name = kb_main[i].String();
            SDL_Keycode key_code = SDL_GetKeyFromName(key_name.c_str());

            if (key_code != SDLK_UNKNOWN)
            {
                code = key_code;
            }
            else
            {
                if (KEY_MOD_MAP.count(key_name))
                {
                    mod |= KEY_MOD_MAP.at(key_name);
                }
                else
                {
                    LOG_ERROR(ERR_MED, "Invalid key mod '%s'!", key_name.c_str());
                    return;
                }
            }
        }
    }

    bool has_alt = key_binding.Contains("alt");
    if (has_alt)
    {
        const GonObject& kb_alt = key_binding["alt"];
        for (int i=0; i<kb_alt.size(); ++i)
        {
            // Convert the key mod and code names into actual values.
            std::string key_name = kb_alt[i].String();
            SDL_Keycode key_code = SDL_GetKeyFromName(key_name.c_str());

            if (key_code != SDLK_UNKNOWN)
            {
                alt_code = key_code;
            }
            else
            {
                if (KEY_MOD_MAP.count(key_name))
                {
                    alt_mod |= KEY_MOD_MAP.at(key_name);
                }
                else
                {
                    LOG_ERROR(ERR_MED, "Invalid key mod '%s'!", key_name.c_str());
                    return;
                }
            }
        }
    }

    // If neither are set then input was malformed or empty.
    if (!code && !mod)
    {
        LOG_ERROR(ERR_MED, "Invalid key binding '%s'!", name);
        return;
    }

    Key_Binding kb = {};
    kb.action   = action;
    kb.has_alt  = has_alt;
    kb.mod      = mod;
    kb.code     = code;
    kb.alt_mod  = alt_mod;
    kb.alt_code = alt_code;

    key_bindings.insert(KB_Pair(name, kb));
}

TEINAPI void internal__load_editor_key_bindings (const GonObject& a, const GonObject& b)
{
    key_bindings.clear();

    internal__add_key_binding(a, b, KB_LEVEL_NEW           , hb_new                     );
    internal__add_key_binding(a, b, KB_LEVEL_OPEN          , hb_load                    );
    internal__add_key_binding(a, b, KB_LEVEL_SAVE          , hb_save                    );
    internal__add_key_binding(a, b, KB_LEVEL_SAVE_AS       , hb_save_as                 );
    internal__add_key_binding(a, b, KB_LEVEL_CLOSE         , close_current_tab          );
    internal__add_key_binding(a, b, KB_LEVEL_CLOSE_ALL     , close_all_tabs             );
    internal__add_key_binding(a, b, KB_LEVEL_RESIZE        , le_resize                  );
    internal__add_key_binding(a, b, KB_UNDO                , hb_undo_action             );
    internal__add_key_binding(a, b, KB_REDO                , hb_redo_action             );
    internal__add_key_binding(a, b, KB_HISTORY_BEGIN       , hb_history_begin           );
    internal__add_key_binding(a, b, KB_HISTORY_END         , hb_history_end             );
    internal__add_key_binding(a, b, KB_CAMERA_ZOOM_OUT     , hb_zoom_out                );
    internal__add_key_binding(a, b, KB_CAMERA_ZOOM_IN      , hb_zoom_in                 );
    internal__add_key_binding(a, b, KB_RUN_GAME            , hb_run_game                );
    internal__add_key_binding(a, b, KB_PREFERENCES         , hb_preferences             );
    internal__add_key_binding(a, b, KB_ABOUT               , hb_about                   );
    internal__add_key_binding(a, b, KB_BUG_REPORT          , hb_bug_report              );
    internal__add_key_binding(a, b, KB_HELP                , hb_help                    );
    internal__add_key_binding(a, b, KB_TOOL_BRUSH          , tb_set_tool_to_brush       );
    internal__add_key_binding(a, b, KB_TOOL_FILL           , tb_set_tool_to_fill        );
    internal__add_key_binding(a, b, KB_TOOL_SELECT         , tb_set_tool_to_select      );
    internal__add_key_binding(a, b, KB_FLIP_H              , tb_flip_level_h            );
    internal__add_key_binding(a, b, KB_FLIP_V              , tb_flip_level_v            );
    internal__add_key_binding(a, b, KB_MIRROR_H_TOGGLE     , tb_toggle_mirror_h         );
    internal__add_key_binding(a, b, KB_MIRROR_V_TOGGLE     , tb_toggle_mirror_v         );
    internal__add_key_binding(a, b, KB_GPAK_UNPACK         , hb_gpak_unpack             );
    internal__add_key_binding(a, b, KB_GPAK_PACK           , hb_gpak_pack               );
    internal__add_key_binding(a, b, KB_TOGGLE_ENTITY       , tb_toggle_entity           );
    internal__add_key_binding(a, b, KB_TOGGLE_GUIDES       , tb_toggle_guides           );
    internal__add_key_binding(a, b, KB_GRID_TOGGLE         , tb_toggle_grid             );
    internal__add_key_binding(a, b, KB_BOUNDS_TOGGLE       , tb_toggle_bounds           );
    internal__add_key_binding(a, b, KB_LAYERS_TOGGLE       , tb_toggle_layer_trans      );
    internal__add_key_binding(a, b, KB_CAMERA_RESET        , tb_reset_camera            );
    internal__add_key_binding(a, b, KB_CLEAR_SELECT        , tb_clear_select            );
    internal__add_key_binding(a, b, KB_DESELECT            , tb_deselect                );
    internal__add_key_binding(a, b, KB_SELECT_ALL          , editor_select_all          );
    internal__add_key_binding(a, b, KB_COPY                , tb_copy                    );
    internal__add_key_binding(a, b, KB_CUT                 , tb_cut                     );
    internal__add_key_binding(a, b, KB_PASTE               , editor_paste               );
    internal__add_key_binding(a, b, KB_INCREMENT_TILE      , increment_selected_tile    );
    internal__add_key_binding(a, b, KB_DECREMENT_TILE      , decrement_selected_tile    );
    internal__add_key_binding(a, b, KB_INCREMENT_GROUP     , increment_selected_group   );
    internal__add_key_binding(a, b, KB_DECREMENT_GROUP     , decrement_selected_group   );
    internal__add_key_binding(a, b, KB_INCREMENT_CATEGORY  , increment_selected_category);
    internal__add_key_binding(a, b, KB_DECREMENT_CATEGORY  , decrement_selected_category);
    internal__add_key_binding(a, b, KB_TOGGLE_LAYER_ACTIVE , toggle_layer_active        );
    internal__add_key_binding(a, b, KB_TOGGLE_LAYER_TAG    , toggle_layer_tag           );
    internal__add_key_binding(a, b, KB_TOGGLE_LAYER_OVERLAY, toggle_layer_overlay       );
    internal__add_key_binding(a, b, KB_TOGGLE_LAYER_BACK1  , toggle_layer_back1         );
    internal__add_key_binding(a, b, KB_TOGGLE_LAYER_BACK2  , toggle_layer_back2         );
    internal__add_key_binding(a, b, KB_CATEGORY_BASIC      , jump_to_category_basic     );
    internal__add_key_binding(a, b, KB_CATEGORY_TAG        , jump_to_category_tag       );
    internal__add_key_binding(a, b, KB_CATEGORY_OVERLAY    , jump_to_category_overlay   );
    internal__add_key_binding(a, b, KB_CATEGORY_ENTITY     , jump_to_category_entity    );
    internal__add_key_binding(a, b, KB_CATEGORY_BACK1      , jump_to_category_back1     );
    internal__add_key_binding(a, b, KB_CATEGORY_BACK2      , jump_to_category_back2     );
    internal__add_key_binding(a, b, KB_INCREMENT_TAB       , increment_tab              );
    internal__add_key_binding(a, b, KB_DECREMENT_TAB       , decrement_tab              );
    internal__add_key_binding(a, b, KB_OPEN_RECENT_TAB     , open_recently_closed_tab   );
    internal__add_key_binding(a, b, KB_MOVE_TAB_LEFT       , move_tab_left              );
    internal__add_key_binding(a, b, KB_MOVE_TAB_RIGHT      , move_tab_right             );
    internal__add_key_binding(a, b, KB_LOAD_PREV_LEVEL     , le_load_prev_level         );
    internal__add_key_binding(a, b, KB_LOAD_NEXT_LEVEL     , le_load_next_level         );
}

/* -------------------------------------------------------------------------- */

TEINAPI bool operator== (const Key_Binding& a, const Key_Binding& b)
{
    return ((a.action   == b.action  ) &&
            (a.code     == b.code    ) &&
            (a.mod      == b.mod     ) &&
            (a.alt_code == b.alt_code) &&
            (a.alt_mod  == b.alt_mod ));
}

TEINAPI bool operator!= (const Key_Binding& a, const Key_Binding& b)
{
    return !(a == b);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_editor_key_bindings ()
{
    GonObject gon;
    try
    {
        std::string file_name(make_path_absolute(KEY_BINDINGS_FILE_NAME));
        gon = GonObject::Load(file_name);
    }
    catch (const char* msg)
    {
        LOG_ERROR(ERR_MED, "%s", msg);

        // If we already have key bind data then we just inform the user that the operation
        // failed. Otherwise, we just fallback to using the default application key binds.
        if (!key_bindings.empty())
        {
            LOG_ERROR(ERR_MED, "Failed to reload key bindings data!");
            return false;
        }
        else
        {
            gon = GonObject::LoadFromBuffer(KEY_BINDINGS_FALLBACK);
        }
    }

    // If we reach this point and there are no key binds then we just use the defaults.
    // This could be the case if the key binds failed to load or haven't been modified.
    if (gon.type != GonObject::FieldType::OBJECT)
    {
        gon = GonObject::LoadFromBuffer(KEY_BINDINGS_FALLBACK);
    }

    GonObject gon_fallback = GonObject::LoadFromBuffer(KEY_BINDINGS_FALLBACK);

    internal__load_editor_key_bindings(gon, gon_fallback);
    LOG_DEBUG("Loaded Editor Key Bindings");

    return true;
}

TEINAPI void restore_editor_key_bindings ()
{
    // Load the fallback/default editor key bindings so they can be restored.
    GonObject gon = GonObject::LoadFromBuffer(KEY_BINDINGS_FALLBACK);
    internal__load_editor_key_bindings(gon, gon);
}

/* -------------------------------------------------------------------------- */

TEINAPI void handle_key_binding_events ()
{
    if (!text_box_is_active() || is_window_focused("WINMAIN"))
    {
        // We only care about key press events, anything else is ignored.
        if (main_event.type == SDL_KEYDOWN)
        {
            for (auto it: key_bindings)
            {
                // Check that the appropriate keys and mods are pressed.
                const Key_Binding& kb = it.second;
                if (internal__key_binding_active(kb))
                {
                    // Do not attempt to call a NULL value!
                    if (kb.action) kb.action();
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI const Key_Binding& get_key_binding (std::string name)
{
    return key_bindings[name];
}

/* -------------------------------------------------------------------------- */

TEINAPI std::string get_key_binding_main_string (const Key_Binding& kb)
{
    return internal__get_key_binding_string(kb.code, kb.mod);
}

TEINAPI std::string get_key_binding_main_string (std::string name)
{
    return get_key_binding_main_string(get_key_binding(name));
}

TEINAPI std::string get_key_binding_alt_string (const Key_Binding& kb)
{
    // There is no alternative key binding for this particular key.
    if (!kb.alt_code && !kb.alt_mod) return std::string();
    return internal__get_key_binding_string(kb.alt_code, kb.alt_mod);
}

TEINAPI std::string get_key_binding_alt_string (std::string name)
{
    return get_key_binding_alt_string(get_key_binding(name));
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_key_binding_active (std::string name)
{
    if (text_box_is_active() || !is_window_focused("WINMAIN")) return false;

    // If the key binding doesn't exist then it can't be active.
    if (!key_bindings.count(name))
    {
        LOG_ERROR(ERR_MIN, "No key binding with name '%s'!", name.c_str());
        return false;
    }

    return internal__key_binding_active(key_bindings.at(name));
}

TEINAPI bool is_key_mod_state_active (int mod)
{
    if (text_box_is_active() | !is_window_focused("WINMAIN")) return false;

    // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
    int curr_mod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));
    // We do not care whether the right or left variants have been pressed.
    if (curr_mod&KMOD_LCTRL  || curr_mod&KMOD_RCTRL ) curr_mod |= KMOD_CTRL;
    if (curr_mod&KMOD_LALT   || curr_mod&KMOD_RALT  ) curr_mod |= KMOD_ALT;
    if (curr_mod&KMOD_LSHIFT || curr_mod&KMOD_RSHIFT) curr_mod |= KMOD_SHIFT;
    if (curr_mod&KMOD_LGUI   || curr_mod&KMOD_RGUI  ) curr_mod |= KMOD_GUI;

    return (curr_mod == mod);
}

TEINAPI bool is_key_code_active (int code)
{
    int key_count; // So we know the length of the returned array.
    const u8* key_state = SDL_GetKeyboardState(&key_count);
    // Need to convert the keycode to a scancode for this.
    SDL_Scancode scan = SDL_GetScancodeFromKey(code);
    return (scan < key_count) ? key_state[scan] : false;
}

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
