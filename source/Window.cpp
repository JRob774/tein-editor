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

    gWindow.cachedWidth = gSettings.windowWidth;
    gWindow.cachedHeight = gSettings.windowHeight;

    gWindow.window = SDL_CreateWindow(gWindowTitle, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        gWindow.cachedWidth,gWindow.cachedHeight, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN);
    if (!gWindow.window) {
        LogSystemMessage("Failed to create window: %s", SDL_GetError());
        return false;
    }

    gWindow.context = SDL_GL_CreateContext(gWindow.window);
    if (!gWindow.context) {
        LogSystemMessage("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        LogSystemMessage("Failed to load OpenGL procedures!");
        return false;
    }

    SDL_SetWindowMinimumSize(gWindow.window, gWindowMinimumWidth,gWindowMinimumHeight);

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
    SDL_GL_DeleteContext(gWindow.context);
    SDL_DestroyWindow(gWindow.window);
}

EditorAPI void RefreshWindow ()
{
    SDL_GL_SwapWindow(gWindow.window);
}

EditorAPI void ShowWindow ()
{
    SDL_ShowWindow(gWindow.window);
}
EditorAPI void HideWindow ()
{
    SDL_HideWindow(gWindow.window);
}

EditorAPI void EnableWindowFullscreen (bool enable)
{
    if (enable) SDL_GetWindowSize(gWindow.window, &gWindow.cachedWidth, &gWindow.cachedHeight);
    gWindow.fullscreen = enable;
    SDL_SetWindowFullscreen(gWindow.window, (gWindow.fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
EditorAPI bool IsWindowFullscreen ()
{
    return gWindow.fullscreen;
}

EditorAPI void SetWindowSize (int width, int height)
{
    SDL_SetWindowSize(gWindow.window, width, height);
}

EditorAPI void SetWindowTitle (std::string title)
{
    SDL_SetWindowTitle(gWindow.window, title.c_str());
}

EditorAPI int GetWindowWidth ()
{
    int width;
    SDL_GetWindowSize(gWindow.window, &width,NULL);
    return width;
}
EditorAPI int GetWindowHeight ()
{
    int height;
    SDL_GetWindowSize(gWindow.window, NULL,&height);
    return height;
}

EditorAPI int GetCachedWindowWidth ()
{
    if (gWindow.fullscreen) return gWindow.cachedWidth;
    return GetWindowWidth();
}
EditorAPI int GetCachedWindowHeight ()
{
    if (gWindow.fullscreen) return gWindow.cachedHeight;
    return GetWindowHeight();
}

EditorAPI SDL_Window* GetInternalWindow ()
{
    return gWindow.window;
}
EditorAPI SDL_GLContext GetInternalContext ()
{
    return gWindow.context;
}
