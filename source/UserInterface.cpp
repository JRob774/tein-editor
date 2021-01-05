EditorAPI bool InitUi ()
{
    PushLogSystem("debug");
    Defer { PopLogSystem(); };

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext()) { // We're assuming NULL means failure, but can't find any docs on it?
        LogSystemMessage("Failed to create ImGui context!");
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsResizeFromEdges = true;
    // io.ConfigViewportsNoTaskBarIcon = true; // @Incomplete: We want this but there's no way to raise all viewports when focusing on the main window.
                                               // We can probably look into adding that functionality to the SDL layer at some point (and window icons).

    // @Incomplete: We want to design our own light and dark themes and also allow the editor user to specify custom themes.
    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;

    // Setup Platform/Renderer backends.
    if (!ImGui_ImplSDL2_InitForOpenGL(GetInternalWindow(), GetInternalContext())) {
        LogSystemMessage("Failed to initialize SDL backend for ImGui!");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        LogSystemMessage("Failed to initialize OpenGL backend for ImGui!");
        return false;
    }

    // Load fonts.
    io.Fonts->AddFontDefault();
    // ...

    return true;
}

EditorAPI void QuitUi ()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

EditorAPI void HandleUiEvents (const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

EditorAPI void BeginUiFrame ()
{
    ImGuiIO& io = ImGui::GetIO();

    // Start the Dear ImGui frame.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(GetInternalWindow());
    ImGui::NewFrame();

    // Only allow multiple viewports when not in fullscreen.
    if (!IsWindowFullscreen()) io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    else io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

    // ImGui::PushFont(iUi.font);

    // A lot of this code is taken from the example docking app in imgui_demo.cpp.
    // It allows for our whole main window to be used as the initial docking node.

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable
    // into, because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    windowFlags |= ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoNavFocus;
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask ImGui::Begin() to not render a background.
    windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0)); // Remove padding.
    bool mainOpen = true;
    ImGui::Begin("Application", &mainOpen, windowFlags);
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2); // This seems to be needed to remove padding if the dock space is the full window (which it is).

    // DockSpace
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;
        ImGuiID dockSpaceID = ImGui::GetID("AppDockSpace");
        ImGui::DockSpace(dockSpaceID, ImVec2(0,0), dockSpaceFlags);
    }

    // MenuBar
    if (ImGui::BeginMenuBar()) {
        DoEditorMenuBar();
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

EditorAPI void EndUiFrame ()
{
    // ImGui::PopFont();

    // Rendering.
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and render additional platform Windows (platform functions may change the current
    // OpenGL context, so we save/restore it to make it easier to paste this code elsewhere).
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backupCurrentWindow = SDL_GL_GetCurrentWindow();
        SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
    }
}
