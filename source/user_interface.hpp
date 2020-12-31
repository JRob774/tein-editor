// These colors are filled with the appropriate values based on whether the
// user would prefer to use the dark or light theme for the level editor.

static Vec4 gUiColorBlack;
static Vec4 gUiColorExDark;
static Vec4 gUiColorDark;
static Vec4 gUiColorMedDark;
static Vec4 gUiColorMedium;
static Vec4 gUiColorMedLight;
static Vec4 gUiColorLight;
static Vec4 gUiColorExLight;
static Vec4 gUiColorWhite;

static constexpr U32 gUiCursorBlinkInterval = 700;

// The function type that is called by various UI widgets when triggered.
typedef void (*UiAction)(void);

enum UiFlag: U32
{
    UI_NONE       = 0x00000000,
    UI_LOCKED     = 0x00000001,
    UI_INACTIVE   = 0x00000002,
    UI_HIGHLIGHT  = 0x00000004,
    UI_DARKEN     = 0x00000008,
    UI_TOOLTIP    = 0x00000010,
    UI_SINGLE     = 0x00000020,
    // Flags for what type of characters can be written into a text box.
    UI_ALPHABETIC = 0x00000040,
    UI_NUMERIC    = 0x00000080,
    UI_ALPHANUM   = 0x00000100,
    UI_FILEPATH   = 0x00000200
};

GenerateEnumBitflagOperators(UiFlag);

enum UiAlign
{
    UI_ALIGN_CENTER,
    UI_ALIGN_LEFT,
    UI_ALIGN_RIGHT,
    UI_ALIGN_TOP,
    UI_ALIGN_BOTTOM
};

enum UiDir
{
    UI_DIR_UP,
    UI_DIR_RIGHT,
    UI_DIR_DOWN,
    UI_DIR_LEFT
};

static bool gShouldPushUiRedrawEvent;

TEINAPI bool InitUiSystem   ();
TEINAPI void LoadUiTheme    ();
TEINAPI void ResetUiState   ();
TEINAPI void HandleUiEvents ();

TEINAPI bool IsUiLight ();

TEINAPI Vec2 UiGetRelativeMouse ();

TEINAPI bool MouseInUiBoundsXYWH (float x, float y, float w, float h);
TEINAPI bool MouseInUiBoundsXYWH (Quad b);

TEINAPI void SetUiTexture (Texture* texture);
TEINAPI void SetUiFont    (Font* font);

TEINAPI bool IsThereAHotUiElement ();
TEINAPI bool IsThereaHitUiElement ();

TEINAPI void DeselectActiveTextBox (std::string& text, std::string defaultText);
TEINAPI void DeselectActiveTextBox ();

TEINAPI bool TextBoxIsActive ();

TEINAPI bool HotkeyIsActive             ();
TEINAPI void DeselectActiveHotkeyRebind ();

TEINAPI void BeginPanel (float x, float y, float w, float h, UiFlag flags, Vec4 color = Vec4(0,0,0,0));
TEINAPI void BeginPanel (Quad bounds, UiFlag flags, Vec4 c = Vec4(0,0,0,0));

TEINAPI bool BeginClickPanel (UiAction action, float w, float h, UiFlag flags, std::string info = "");

TEINAPI void EndPanel ();

TEINAPI float GetPanelWidth ();
TEINAPI float GetPanelHeight ();

TEINAPI Vec2 GetPanelOffset ();
TEINAPI Vec2 GetPanelCursor ();

TEINAPI void DisablePanelCursorAdvance ();
TEINAPI void EnablePanelCursorAdvance  ();

TEINAPI void AdvancePanelCursor (float advance);

TEINAPI void SetPanelCursor    (Vec2* cursor);
TEINAPI void SetPanelCursorDir (UiDir dir);

TEINAPI void   SetPanelFlags (UiFlag flags);
TEINAPI UiFlag GetPanelFlags ();

TEINAPI float CalculateTextButtonWidth (std::string text);

TEINAPI bool DoImageButton (UiAction action, float w, float h, UiFlag flags, const Quad* clip = NULL, std::string info = "", std::string kb = "", std::string name = "");
TEINAPI bool DoTextButton  (UiAction action, float w, float h, UiFlag flags, std::string text,        std::string info = "", std::string kb = "", std::string name = "");
TEINAPI bool DoTextButton  (UiAction action,          float h, UiFlag flags, std::string text,        std::string info = "", std::string kb = "", std::string name = "");

TEINAPI void DoLabel (UiAlign horz, UiAlign vert, float w, float h, std::string text, Vec4 bgColor = Vec4(0,0,0,0));
TEINAPI void DoLabel (UiAlign horz, UiAlign vert,          float h, std::string text, Vec4 bgColor = Vec4(0,0,0,0));

TEINAPI void DoLabelHyperlink (UiAlign horz, UiAlign vert, float w, float h, std::string text, std::string link, std::string href, Vec4 bgColor = Vec4(0,0,0,0));

// Doesn't care about the current UI font and uses the current editor font instead!
TEINAPI void  DoMarkdown        (float w, float h, std::string text);
TEINAPI float GetMarkdownHeight (float w,          std::string text);

TEINAPI void DoTextBox        (float w, float h, UiFlag flags, std::string& text,                                      std::string defaultText = "", UiAlign hAlign = UI_ALIGN_RIGHT);
TEINAPI void DoTextBoxLabeled (float w, float h, UiFlag flags, std::string& text, float labelWidth, std::string label, std::string defaultText = "", UiAlign hAlign = UI_ALIGN_RIGHT);

TEINAPI void DoHotkeyRebindMain (float w, float h, UiFlag flags, KeyBinding& kb);
TEINAPI void DoHotkeyRebindAlt  (float w, float h, UiFlag flags, KeyBinding& kb);

TEINAPI void DoIcon (float w, float h, Texture& texture, const Quad* clip = NULL);
TEINAPI void DoQuad (float w, float h, Vec4 color);

TEINAPI void DoSeparator (float size);

TEINAPI void DoScrollbar (Quad bounds,                        float contentHeight, float& scrollOffset);
TEINAPI void DoScrollbar (float x, float y, float w, float h, float contentHeight, float& scrollOffset);

TEINAPI void BeginPanelGradient (float x, float y, float w, float h, UiFlag flags, Vec4 leftColor = Vec4(0,0,0,0), Vec4 rightColor = Vec4(0,0,0,0));
TEINAPI void BeginPanelGradient (Quad bounds,                        UiFlag flags, Vec4 leftColor = Vec4(0,0,0,0), Vec4 rightColor = Vec4(0,0,0,0));

TEINAPI bool BeginClickPanelGradient (UiAction action, float w, float h, UiFlag flags, std::string info = "");

TEINAPI bool DoImageButtonGradient (UiAction action, float w, float h, UiFlag flags, const Quad* clip = NULL, std::string info = "", std::string kb = "", std::string name = "");
