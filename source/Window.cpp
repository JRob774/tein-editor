Internal struct
{
    SDL_Window* window;
    SDL_GLContext context;
    int cachedWidth; // The size of the non-fullscreen window.
    int cachedHeight;
    bool fullscreen;

} iWindow;

EditorAPI bool InitWindow ()
{
    PushLogSystem("window");
    Defer { PopLogSystem(); };

    // The SDL docs say that this should be done before creation of the window!
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    iWindow.cachedWidth = gSettings.windowWidth;
    iWindow.cachedHeight = gSettings.windowHeight;

    iWindow.window = SDL_CreateWindow(gWindowTitle, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        iWindow.cachedWidth,iWindow.cachedHeight, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);
    if (!iWindow.window) {
        LogSystemMessage("Failed to create window: %s", SDL_GetError());
        return false;
    }

    iWindow.context = SDL_GL_CreateContext(iWindow.window);
    if (!iWindow.context) {
        LogSystemMessage("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        LogSystemMessage("Failed to load OpenGL procedures!");
        return false;
    }

    SDL_SetWindowMinimumSize(iWindow.window, gWindowMinimumWidth,gWindowMinimumHeight);

    EnableWindowFullscreen(gSettings.fullscreen);

    if (SDL_GL_SetSwapInterval(-1) < 0) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            LogSystemMessage("Failed to enable VSync!");
        }
    }

    return true;
}

EditorAPI void QuitWindow ()
{
    SDL_GL_DeleteContext(iWindow.context);
    SDL_DestroyWindow(iWindow.window);
}

EditorAPI void RefreshWindow ()
{
    SDL_GL_SwapWindow(iWindow.window);
}

EditorAPI void ShowWindow ()
{
    SDL_ShowWindow(iWindow.window);
}
EditorAPI void HideWindow ()
{
    SDL_HideWindow(iWindow.window);
}

EditorAPI void EnableWindowFullscreen (bool enable)
{
    if (enable) SDL_GetWindowSize(iWindow.window, &iWindow.cachedWidth, &iWindow.cachedHeight);
    iWindow.fullscreen = enable;
    SDL_SetWindowFullscreen(iWindow.window, (iWindow.fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
EditorAPI bool IsWindowFullscreen ()
{
    return iWindow.fullscreen;
}

EditorAPI void SetWindowSize (int width, int height)
{
    SDL_SetWindowSize(iWindow.window, width, height);
}

EditorAPI void SetWindowTitle (std::string title)
{
    SDL_SetWindowTitle(iWindow.window, title.c_str());
}

EditorAPI int GetWindowWidth ()
{
    int width;
    SDL_GetWindowSize(iWindow.window, &width,NULL);
    return width;
}
EditorAPI int GetWindowHeight ()
{
    int height;
    SDL_GetWindowSize(iWindow.window, NULL,&height);
    return height;
}

EditorAPI int GetCachedWindowWidth ()
{
    if (iWindow.fullscreen) return iWindow.cachedWidth;
    return GetWindowWidth();
}
EditorAPI int GetCachedWindowHeight ()
{
    if (iWindow.fullscreen) return iWindow.cachedHeight;
    return GetWindowHeight();
}

EditorAPI SDL_Window* GetInternalWindow ()
{
    return iWindow.window;
}
EditorAPI SDL_GLContext GetInternalContext ()
{
    return iWindow.context;
}
