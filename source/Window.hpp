Global constexpr const char* gWindowTitle = "The End is Nigh: Editor";

Global constexpr int gWindowStartWidth = 1280;
Global constexpr int gWindowStartHeight = 720;
Global constexpr int gWindowMinimumWidth = 320;
Global constexpr int gWindowMinimumHeight = 240;

struct Window
{
    SDL_Window* window;
    SDL_GLContext context;
    int cachedWidth; // The size of the non-fullscreen window.
    int cachedHeight;
    bool fullscreen;
};

Global Window gWindow;

// Interface

EditorAPI bool InitWindow ();
EditorAPI void QuitWindow ();

EditorAPI void RefreshWindow ();

EditorAPI void ShowWindow ();
EditorAPI void HideWindow ();

EditorAPI void EnableWindowFullscreen (bool enable);
EditorAPI bool IsWindowFullscreen ();

EditorAPI void SetWindowSize (int width, int height);
EditorAPI void SetWindowTitle (std::string title);

EditorAPI int GetWindowWidth ();
EditorAPI int GetWindowHeight ();

EditorAPI int GetCachedWindowWidth ();
EditorAPI int GetCachedWindowHeight ();

// @Improve: We want to remove any SDL definitions from the API side of the engine, this makes
// it more platform-agnostic and allows for easier implementation of other platform backends.
EditorAPI SDL_Window* GetInternalWindow ();
EditorAPI SDL_GLContext GetInternalContext ();
