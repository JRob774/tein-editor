#pragma once

//
// GLOBAL WINDOW FUNCTIONALITY
//

static constexpr const char* gMainWindowTitle = "The End is Nigh: Editor";
static constexpr int         gMainWindowX     = SDL_WINDOWPOS_CENTERED;
static constexpr int         gMainWindowY     = SDL_WINDOWPOS_CENTERED;
static constexpr int         gMainWindowBaseW = 1280;
static constexpr int         gMainWindowBaseH =  722; // +2 so the info panel doesn't shift the layer panel!
static constexpr int         gMainWindowMinW  =  600;
static constexpr int         gMainWindowMinH  =  320;
static constexpr U32         gMainWindowFlags = SDL_WINDOW_RESIZABLE;

static constexpr float gWindowBorder = 4.0f;

typedef U32 WindowID;
struct Window;

TEINAPI bool InitWindow ();
TEINAPI void QuitWindow ();

TEINAPI void HandleWindowEvents ();

TEINAPI void SetMainWindowSubtitle (std::string subtitle);

TEINAPI void ShowMainWindow ();

TEINAPI Window&     GetFocusedWindow    ();
TEINAPI Window&     GetWindowFromName   (std::string name);
TEINAPI Window&     GetWindowFromID     (WindowID id);
TEINAPI WindowID    GetWindowIDFromName (std::string name);
TEINAPI std::string GetWindowNameFromID (WindowID id);

TEINAPI bool IsAWindowResizing ();

//
// INDIVIDUAL WINDOW FUNCTIONALITY
//

typedef void(*WindowResizeCallback)(void);
typedef void(*WindowCloseCallback)(void);

struct Window
{
    WindowResizeCallback resizeCallback;
    WindowCloseCallback closeCallback;

    SDL_Window* window;
    WindowID id;

    bool focus;
    bool mouse;

    float width;
    float height;
};

TEINAPI bool RegisterWindow (std::string name, std::string title, int x, int y, int w, int h, int minW = 0, int minH = 0, U32 flags = 0);

TEINAPI bool IsWindowHidden  (std::string name);
TEINAPI bool IsWindowFocused (std::string name);

TEINAPI void ShowWindow  (std::string name);
TEINAPI void RaiseWindow (std::string name);
TEINAPI void HideWindow  (std::string name);

TEINAPI void SetWindowTitle   (std::string name, std::string title);
TEINAPI void SetWindowMinSize (std::string name, int w, int h);
TEINAPI void SetWindowMaxSize (std::string name, int w, int h);
TEINAPI void SetWindowPos     (std::string name, int x, int y);
TEINAPI void SetWindowSize    (std::string name, int w, int h);
TEINAPI void SetWindowChild   (std::string name);
