#pragma once

static constexpr const char* gKeyBindingsFileName = "editor_settings.txt";

// The names for all of the key bindings found in the user settings file.
// We store them here so that if we ever want to change any of the names
// then we only have to update them here rather than everywhere they are
// referenced throughout the program's source code -- would be annoying!

static constexpr const char* gKbLevelNew           = "level_new";
static constexpr const char* gKbLevelOpen          = "level_open";
static constexpr const char* gKbLevelSave          = "level_save";
static constexpr const char* gKbLevelSaveAs        = "level_save_as";
static constexpr const char* gKbLevelClose         = "level_close";
static constexpr const char* gKbLevelCloseAll      = "level_close_all";
static constexpr const char* gKbLevelResize        = "level_resize";
static constexpr const char* gKbUndo               = "undo";
static constexpr const char* gKbRedo               = "redo";
static constexpr const char* gKbHistoryBegin       = "history_begin";
static constexpr const char* gKbHistoryEnd         = "history_end";
static constexpr const char* gKbCameraZoomOut      = "camera_zoom_out";
static constexpr const char* gKbCameraZoomIn       = "camera_zoom_in";
static constexpr const char* gKbRunGame            = "run_game";
static constexpr const char* gKbPreferences        = "preferences";
static constexpr const char* gKbAbout              = "about";
static constexpr const char* gKbBugReport          = "bug_report";
static constexpr const char* gKbHelp               = "help";
static constexpr const char* gKbToolBrush          = "tool_brush";
static constexpr const char* gKbToolFill           = "tool_fill";
static constexpr const char* gKbToolSelect         = "tool_select";
static constexpr const char* gKbFlipH              = "flip_h";
static constexpr const char* gKbFlipV              = "flip_v";
static constexpr const char* gKbMirrorHToggle      = "mirror_h_toggle";
static constexpr const char* gKbMirrorVToggle      = "mirror_v_toggle";
static constexpr const char* gKbGpakUnpack         = "gpak_unpack";
static constexpr const char* gKbGpakPack           = "gpak_pack";
static constexpr const char* gKbToggleEntity       = "toggle_large_entity";
static constexpr const char* gKbToggleGuides       = "toggle_entity_guides";
static constexpr const char* gKbGridToggle         = "grid_toggle";
static constexpr const char* gKbBoundsToggle       = "bounds_toggle";
static constexpr const char* gKbLayersToggle       = "layer_trans_toggle";
static constexpr const char* gKbCameraReset        = "camera_reset";
static constexpr const char* gKbClearSelect        = "clear_select";
static constexpr const char* gKbDeselect           = "deselect";
static constexpr const char* gKbSelectAll          = "select_all";
static constexpr const char* gKbCopy               = "copy";
static constexpr const char* gKbCut                = "cut";
static constexpr const char* gKbPaste              = "paste";
static constexpr const char* gKbIncrementTile      = "increment_tile";
static constexpr const char* gKbDecrementTile      = "decrement_tile";
static constexpr const char* gKbIncrementGroup     = "increment_group";
static constexpr const char* gKbDecrementGroup     = "decrement_group";
static constexpr const char* gKbIncrementCategory  = "increment_category";
static constexpr const char* gKbDecrementCategory  = "decrement_category";
static constexpr const char* gKbToggleLayerActive  = "toggle_layer_active";
static constexpr const char* gKbToggleLayerTag     = "toggle_layer_tag";
static constexpr const char* gKbToggleLayerOverlay = "toggle_layer_overlay";
static constexpr const char* gKbToggleLayerBack1   = "toggle_layer_back1";
static constexpr const char* gKbToggleLayerBack2   = "toggle_layer_back2";
static constexpr const char* gKbCategoryBasic      = "category_basic";
static constexpr const char* gKbCategoryTag        = "category_tag";
static constexpr const char* gKbCategoryOverlay    = "category_overlay";
static constexpr const char* gKbCategoryEntity     = "category_entity";
static constexpr const char* gKbCategoryBack1      = "category_back1";
static constexpr const char* gKbCategoryBack2      = "category_back2";
static constexpr const char* gKbIncrementTab       = "increment_tab";
static constexpr const char* gKbDecrementTab       = "decrement_tab";
static constexpr const char* gKbMoveTabLeft        = "move_tab_left";
static constexpr const char* gKbMoveTabRight       = "move_tab_right";
static constexpr const char* gKbOpenRecentTab      = "open_recent_tab";
static constexpr const char* gKbLoadPrevLevel      = "load_prev_level";
static constexpr const char* gKbLoadNextLevel      = "load_next_level";

typedef void(*KBAction)(void);

struct KeyBinding
{
    KBAction action;
    bool hasAlt;
    int mod;
    int code;
    int altMod;
    int altCode;
};

TEINAPI inline bool operator== (const KeyBinding& a, const KeyBinding& b);
TEINAPI inline bool operator!= (const KeyBinding& a, const KeyBinding& b);

static std::map<std::string,KeyBinding> gKeyBindings;

TEINAPI bool LoadEditorKeyBindings    ();
TEINAPI void RestoreEditorKeyBindings ();

TEINAPI void HandleKeyBindingEvents   ();

TEINAPI const KeyBinding& GetKeyBinding (std::string name);

TEINAPI std::string GetKeyBindingMainString (const KeyBinding& kb);
TEINAPI std::string GetKeyBindingMainString (std::string name);
TEINAPI std::string GetKeyBindingAltString  (const KeyBinding& kb);
TEINAPI std::string GetKeyBindingAltString  (std::string name);

TEINAPI bool IsKeyBindingActive   (std::string name);
TEINAPI bool IsKeyModStateActive  (int mod);
TEINAPI bool IsKeyCodeActive      (int code);
