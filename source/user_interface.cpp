/*******************************************************************************
 * Implementation of the immediate-mode graphical-user-interface facilities.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static const Vec4 UI_D_COLOR_BLACK     = {  .00f,  .00f, 0.00f, 1 };
static const Vec4 UI_D_COLOR_EX_DARK   = {  .20f,  .20f, 0.20f, 1 };
static const Vec4 UI_D_COLOR_DARK      = {  .20f,  .20f, 0.20f, 1 };
static const Vec4 UI_D_COLOR_MED_DARK  = {  .25f,  .25f, 0.25f, 1 };
static const Vec4 UI_D_COLOR_MEDIUM    = {  .33f,  .33f, 0.33f, 1 };
static const Vec4 UI_D_COLOR_MED_LIGHT = {  .43f,  .43f, 0.43f, 1 };
static const Vec4 UI_D_COLOR_LIGHT     = {  .60f,  .60f, 0.60f, 1 };
static const Vec4 UI_D_COLOR_EX_LIGHT  = {  .90f,  .90f, 0.90f, 1 };
static const Vec4 UI_D_COLOR_WHITE     = { 1.00f, 1.00f, 1.00f, 1 };
static const Vec4 UI_L_COLOR_BLACK     = {  .15f,  .15f,  .15f, 1 };
static const Vec4 UI_L_COLOR_EX_DARK   = {  .35f,  .35f,  .35f, 1 };
static const Vec4 UI_L_COLOR_DARK      = {  .55f,  .55f,  .55f, 1 };
static const Vec4 UI_L_COLOR_MED_DARK  = {  .70f,  .70f,  .70f, 1 };
static const Vec4 UI_L_COLOR_MEDIUM    = {  .80f,  .80f,  .80f, 1 };
static const Vec4 UI_L_COLOR_MED_LIGHT = {  .93f,  .93f,  .93f, 1 };
static const Vec4 UI_L_COLOR_LIGHT     = {  .93f,  .93f,  .93f, 1 };
static const Vec4 UI_L_COLOR_EX_LIGHT  = {  .96f,  .96f,  .96f, 1 };
static const Vec4 UI_L_COLOR_WHITE     = { 1.00f, 1.00f, 1.00f, 1 };

/* -------------------------------------------------------------------------- */

struct Panel
{
    Quad absolute_bounds; // Panel position and size on the window.
    Quad viewport;        // Viewport clipped inside parent.
    Vec2 relative_offset; // Panel position relative to its viewport.

    UI_Flag flags;        // Flags that get applied to a panel's content.

    Vec2*  cursor;
    UI_Dir cursor_dir;

    bool cursor_advance_enabled;
};

/* -------------------------------------------------------------------------- */

typedef U32 UI_ID;

static constexpr UI_ID UI_INVALID_ID = UINT32_MAX;

static std::stack<Panel> ui_panels;

static UI_ID ui_current_id;

static UI_ID ui_hot_id;
static UI_ID ui_hit_id;

static UI_ID  ui_active_text_box;
static UI_ID  ui_hot_text_box;
static size_t ui_text_box_cursor;

static UI_ID ui_hot_hyperlink;

static UI_ID ui_active_hotkey_rebind;

static Texture* ui_texture;
static Font*    ui_font;

static Vec2 ui_mouse_relative;

static bool ui_mouse_up;
static bool ui_mouse_down;

static bool ui_mouse_r_up;
static bool ui_mouse_r_down;

static bool ui_is_light;

// We also store the active window ID at the time of the tab press so that the
// system does not move from window-to-window when the user presseses tab.
static WindowID ui_text_box_tab_window_id;
static bool      ui_make_next_text_box_active;
static bool      ui_tab_handled;

enum class UI_Text_Event_Type: U32 { TEXT, KEY };

struct UI_Text_Event
{
    UI_Text_Event_Type type;

    std::string text;
    SDL_Keycode key;
};

static std::vector<UI_Text_Event> ui_text_events;

static SDL_TimerID ui_cursor_blink_timer;
static bool        ui_cursor_visible;

/* -------------------------------------------------------------------------- */

TEINAPI U32 internal__cursor_blink_callback (U32 interval, void* user_data)
{
    PushEditorEvent(EDITOR_EVENT_BLINK_CURSOR, NULL, NULL);
    return interval;
}

TEINAPI bool internal__is_hot ()
{
    return (ui_current_id == ui_hot_id);
}

TEINAPI bool internal__is_hit ()
{
    return (ui_current_id == ui_hit_id);
}

TEINAPI bool internal__is_file_path (char c)
{
    return (std::string("<>\"|?*").find(c) == std::string::npos);
}

TEINAPI Quad internal__get_clipped_bounds (float x, float y, float w, float h)
{
    // Clip the widget's bounds to be within the panel's visible area.
    // This stops the user being able to click on invisible portions.
    //
    // We also make the bounds relative to the window, so that they can
    // properly be compared with the mouse cursor without any issues.
    const Quad& v = get_viewport();
    Quad clipped_bounds;

    clipped_bounds.x1 = std::max(x,        0.0f) + v.x;
    clipped_bounds.y1 = std::max(y,        0.0f) + v.y;
    clipped_bounds.x2 = std::min(x + (w-1), v.w) + v.x;
    clipped_bounds.y2 = std::min(y + (h-1), v.h) + v.y;

    return clipped_bounds;
}

TEINAPI Quad internal__get_clipped_bounds (Quad& p)
{
    // Clip the widget's bounds to be within the panel's visible area.
    // This stops the user being able to click on invisible portions.
    //
    // We also make the bounds relative to the window, so that they can
    // properly be compared with the mouse cursor without any issues.
    const Quad& v = get_viewport();
    Quad clipped_bounds;

    clipped_bounds.x1 = std::max(p.x,          0.0f) + v.x;
    clipped_bounds.y1 = std::max(p.y,          0.0f) + v.y;
    clipped_bounds.x2 = std::min(p.x + (p.w-1), v.w) + v.x;
    clipped_bounds.y2 = std::min(p.y + (p.h-1), v.h) + v.y;

    return clipped_bounds;
}

TEINAPI bool internal__handle_widget (float x, float y, float w, float h, bool locked)
{
    bool result = false;

    // Don't bother handling widgets during resize to avoid ugly redraw stuff.
    if (!IsAWindowResizing())
    {
        if (get_render_target()->focus && get_render_target()->mouse)
        {
            Quad clipped_bounds = internal__get_clipped_bounds(x, y, w, h);
            Vec2 mouse = GetMousePos();

            // Determine the hot and active states for the global UI context.
            bool inside = PointInBoundsXYXY(mouse, clipped_bounds);
            if (!locked && ui_hit_id == ui_current_id)
            {
                if (ui_mouse_up)
                {
                    if (ui_hot_id == ui_current_id)
                    {
                        result = true;
                    }
                    ui_hit_id = UI_INVALID_ID;
                }
            }
            if (ui_hot_id == ui_current_id)
            {
                if (!locked && inside)
                {
                    if (ui_mouse_down)
                    {
                        ui_hit_id = ui_current_id;
                    }
                }
                else
                {
                    ui_hot_id = UI_INVALID_ID;
                }
            }
            if (inside)
            {
                ui_hot_id = ui_current_id;
            }
        }
    }

    // If true then the widget being checked was activated this frame.
    return result;
}

TEINAPI bool internal__handle_widget (Quad b, bool locked)
{
    return internal__handle_widget(b.x, b.y, b.w, b.h, locked);
}

TEINAPI Vec2& internal__get_cursor_ref (Panel& panel)
{
    assert(panel.cursor);
    return *panel.cursor;
}

TEINAPI Vec2 internal__get_cursor (Panel& panel)
{
    assert(panel.cursor);
    return *panel.cursor;
}

TEINAPI Vec2 internal__get_relative_cursor (Panel& panel)
{
    Vec2 cur = internal__get_cursor(panel);

    cur.x += panel.relative_offset.x;
    cur.y += panel.relative_offset.y;

    return cur;
}

TEINAPI void internal__draw_separator (Vec2 cursor, UI_Dir dir, float w, float h, Vec4 color)
{
    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x;
    float y2 = cursor.y;

    switch (dir)
    {
    case (UI_DIR_UP   ): { x1+=1; x2+=(w-1); y1+=1; y2+= 1;    } break;
    case (UI_DIR_RIGHT): { x1+=w; x2+= w;    y1+=1; y2+=(h-1); } break;
    case (UI_DIR_DOWN ): { x1+=1; x2+=(w-1); y1+=h; y2+= h;    } break;
    case (UI_DIR_LEFT ): { x1+=1; x2+= 1;    y1+=1; y2+=(h-1); } break;
    }

    set_draw_color(color);
    draw_line(x1, y1, x2, y2);
}

TEINAPI void internal__advance_ui_cursor_start (Panel& panel, float w, float h)
{
    if (!panel.cursor_advance_enabled) return;

    Vec2& cur = internal__get_cursor_ref(panel);

    switch (panel.cursor_dir)
    {
    case(UI_DIR_UP  ): cur.y -= h; break;
    case(UI_DIR_LEFT): cur.x -= w; break;
    }
}

TEINAPI void internal__advance_ui_cursor_end (Panel& panel, float w, float h)
{
    if (!panel.cursor_advance_enabled) return;

    Vec2& cur = internal__get_cursor_ref(panel);

    switch (panel.cursor_dir)
    {
    case(UI_DIR_RIGHT): cur.x += w; break;
    case(UI_DIR_DOWN ): cur.y += h; break;
    }
}

TEINAPI void internal__align_text (UI_Align horz, UI_Align vert, float& x, float& y, float tw, float th, float w, float h)
{
    // Determine how to place the text based on alignment.
    switch (horz)
    {
    case (UI_ALIGN_LEFT  ): /* No need to do anything. */                        break;
    case (UI_ALIGN_RIGHT ): x += roundf( (w-tw));                                break;
    case (UI_ALIGN_CENTER): x += roundf(((w-tw)/2));                             break;
    }
    switch (vert)
    {
    case (UI_ALIGN_TOP   ): y += ui_font->line_gap.at(ui_font->current_pt_size); break;
    case (UI_ALIGN_BOTTOM): y += roundf(((h)  -(th/4)));                         break;
    case (UI_ALIGN_CENTER): y += roundf(((h/2)+(th/4)));                         break;
    }
}

TEINAPI bool internal__is_ui_mouse_down ()
{
    return (get_render_target()->focus) ? ui_mouse_down : false;
}

TEINAPI bool internal__is_ui_mouse_r_down ()
{
    return (get_render_target()->focus) ? ui_mouse_r_down : false;
}

TEINAPI std::string internal__do_markdown_formatting (std::vector<std::string>& lines, float w)
{
    Font& fnt = get_editor_regular_font();

    std::string text;
    for (auto& line: lines)
    {
        if (line.at(0) == '*') // Looks nicer.
        {
            line.at(0) = '>';
        }

        if (get_text_width_scaled(fnt, line) >= w) // Word-wrap.
        {
            float xoff = 0.0f;

            int i = 0;
            int p = 0;

            for (int j=0; j<static_cast<int>(line.length()); ++j)
            {
                xoff += get_glyph_advance(fnt, line.at(j), i, p);

                if (line.at(j) == '\n')
                {
                    xoff = 0.0f;
                }

                if (xoff >= w)
                {
                    for (int k=j; k>=0; --k)
                    {
                        if (line.at(k) == '\n' || k == 0)
                        {
                            line.at(k) = '\n';
                            xoff = 0.0f;
                            j = k;
                            break;
                        }
                        if (line.at(k) == ' ')
                        {
                            line.insert(k, "\n");
                            xoff = 0.0f;
                            break;
                        }
                    }
                }
            }
        }

        text += line + "\n";
    }

    if (text.back() == '\n')
    {
        text.pop_back();
    }

    return text;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool init_ui_system ()
{
    ui_hot_id = UI_INVALID_ID;
    ui_hit_id = UI_INVALID_ID;

    ui_active_text_box = UI_INVALID_ID;
    ui_hot_text_box    = UI_INVALID_ID;
    ui_text_box_cursor = std::string::npos;

    ui_hot_hyperlink = UI_INVALID_ID;

    ui_active_hotkey_rebind = UI_INVALID_ID;

    ui_texture = NULL;
    ui_font    = NULL;

    ui_mouse_relative = Vec2(0,0);

    ui_mouse_up     = false;
    ui_mouse_down   = false;
    ui_mouse_r_up   = false;
    ui_mouse_r_down = false;

    ui_text_box_tab_window_id    = 0;
    ui_make_next_text_box_active = false;
    ui_tab_handled               = false;

    ui_cursor_blink_timer = NULL;
    ui_cursor_visible     = true;

    return true;
}

TEINAPI void load_ui_theme ()
{
    std::string theme = editor_settings.ui_theme;
    if (theme == "dark")
    {
        ui_color_black     = UI_D_COLOR_BLACK;
        ui_color_ex_dark   = UI_D_COLOR_EX_DARK;
        ui_color_dark      = UI_D_COLOR_DARK;
        ui_color_med_dark  = UI_D_COLOR_MED_DARK;
        ui_color_medium    = UI_D_COLOR_MEDIUM;
        ui_color_med_light = UI_D_COLOR_MED_LIGHT;
        ui_color_light     = UI_D_COLOR_LIGHT;
        ui_color_ex_light  = UI_D_COLOR_EX_LIGHT;
        ui_color_white     = UI_D_COLOR_WHITE;
        ui_is_light        = false;
    }
    else
    {
        ui_color_black     = UI_L_COLOR_BLACK;
        ui_color_ex_dark   = UI_L_COLOR_EX_DARK;
        ui_color_dark      = UI_L_COLOR_DARK;
        ui_color_med_dark  = UI_L_COLOR_MED_DARK;
        ui_color_medium    = UI_L_COLOR_MEDIUM;
        ui_color_med_light = UI_L_COLOR_MED_LIGHT;
        ui_color_light     = UI_L_COLOR_LIGHT;
        ui_color_ex_light  = UI_L_COLOR_EX_LIGHT;
        ui_color_white     = UI_L_COLOR_WHITE;
        ui_is_light        = true;
    }

    // Reload cursors because the beam cursor changes based on theme.
    load_editor_cursors();

    // These colors, if default, also depend on the UI theme.
    Vec4 default_background_color = ui_color_light;
    Vec4 default_tile_grid_color = is_ui_light() ? ui_color_black : ui_color_ex_dark;

    if (editor_settings.background_color_defaulted)
    {
        editor_settings.background_color = default_background_color;
    }
    if (editor_settings.tile_grid_color_defaulted)
    {
        editor_settings.tile_grid_color = default_tile_grid_color;
    }
}

TEINAPI void reset_ui_state ()
{
    // Reset the internal UI ID back to the beginning for a new update/cycle.
    ui_current_id = 0;

    ui_text_events.clear();
    ui_tab_handled = false;

    // We do this during every event otherwise we can end up with some weird
    // values provided by SDL_GetRelativeMouseState, so we just cache here.
    int imx, imy; SDL_GetRelativeMouseState(&imx, &imy);
    ui_mouse_relative = IVec2(imx, imy);

    ui_mouse_down   = false;
    ui_mouse_up     = false;
    ui_mouse_r_down = false;
    ui_mouse_r_up   = false;
}

TEINAPI void handle_ui_events ()
{
    bool prev_down = ui_mouse_down;
    bool prev_up = ui_mouse_up;

    switch (main_event.type)
    {
        case (SDL_MOUSEBUTTONDOWN):
        {
            if      (main_event.button.button == SDL_BUTTON_LEFT ) ui_mouse_down   = true;
            else if (main_event.button.button == SDL_BUTTON_RIGHT) ui_mouse_r_down = true;
        } break;
        case (SDL_MOUSEBUTTONUP):
        {
            if      (main_event.button.button == SDL_BUTTON_LEFT ) ui_mouse_up     = true;
            else if (main_event.button.button == SDL_BUTTON_RIGHT) ui_mouse_r_up   = true;
        } break;
        case (SDL_WINDOWEVENT):
        {
            // When focus changes to a new window then the current text box should be deselected.
            // We also do not want the current hit and hot IDs to persist to the new window.
            if (main_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                deselect_active_text_box();
                deselect_active_hotkey_rebind();

                ui_hot_id = UI_INVALID_ID;
                ui_hit_id = UI_INVALID_ID;
            }
        } break;
        case (SDL_USEREVENT):
        {
            if (main_event.user.code == EDITOR_EVENT_BLINK_CURSOR)
            {
                ui_cursor_visible = !ui_cursor_visible;
            }
        } break;
    }

    // Handle events specifically for UI text boxes.
    if (ui_active_text_box != UI_INVALID_ID)
    {
        UI_Text_Event text_event;
        switch (main_event.type)
        {
            case (SDL_TEXTINPUT):
            {
                text_event.type = UI_Text_Event_Type::TEXT;
                text_event.text = main_event.text.text;
                ui_text_events.push_back(text_event);
            } break;
            case (SDL_KEYDOWN):
            {
                text_event.type = UI_Text_Event_Type::KEY;
                text_event.key  = main_event.key.keysym.sym;
                ui_text_events.push_back(text_event);
            } break;
        }
    }
    else
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            // We do this so we can focus on the first text box in the window!
            if (main_event.key.keysym.sym == SDLK_TAB)
            {
                ui_make_next_text_box_active = true;
                ui_text_box_tab_window_id = GetFocusedWindow().id;
            }
        }
    }

    // The UI has not changed state so we don't have to worry about redraws.
    if (prev_down == ui_mouse_down && prev_up == ui_mouse_up) return;

    // This event exists to perform a second redraw of the user interface. Due
    // to the fact we are using an immediate mode GUI, there is a frame delay
    // that causes some gross visual issues due to our program being mainly
    // event-driven. This method of redrawing again prevents the issue easily.
    should_push_ui_redraw_event = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_ui_light ()
{
    return ui_is_light;
}

/* -------------------------------------------------------------------------- */

TEINAPI Vec2 ui_get_relative_mouse ()
{
    return ui_mouse_relative;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool mouse_in_ui_bounds_xywh (float x, float y, float w, float h)
{
    Quad clipped_bounds = internal__get_clipped_bounds(x, y, w, h);
    Vec2 mouse = GetMousePos();
    return PointInBoundsXYXY(mouse, clipped_bounds);
}

TEINAPI bool mouse_in_ui_bounds_xywh (Quad b)
{
    return mouse_in_ui_bounds_xywh(b.x, b.y, b.w, b.h);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_ui_texture (Texture* tex)
{
    ui_texture = tex;
}

TEINAPI void set_ui_font (Font* fnt)
{
    ui_font = fnt;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool is_there_a_hot_ui_element ()
{
    return (ui_hot_id != UI_INVALID_ID);
}

TEINAPI bool is_there_a_hit_ui_element ()
{
    return (ui_hit_id != UI_INVALID_ID);
}

/* -------------------------------------------------------------------------- */

TEINAPI void deselect_active_text_box (std::string& text, std::string default_text)
{
    // If specified and the text box is empty on exit then we assign
    // the content of the text box to be the passed in default value.
    if (!default_text.empty() && !text.length()) text = default_text;
    deselect_active_text_box();
}

TEINAPI void deselect_active_text_box ()
{
    if (ui_cursor_blink_timer)
    {
        SDL_RemoveTimer(ui_cursor_blink_timer);
        ui_cursor_blink_timer = NULL;
    }

    ui_text_box_cursor = std::string::npos;
    ui_active_text_box = UI_INVALID_ID;

    SDL_StopTextInput();
}

/* -------------------------------------------------------------------------- */

TEINAPI bool text_box_is_active ()
{
    return (ui_active_text_box != UI_INVALID_ID);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool hotkey_is_active ()
{
    return (ui_active_hotkey_rebind != UI_INVALID_ID);
}

TEINAPI void deselect_active_hotkey_rebind ()
{
    ui_active_hotkey_rebind = UI_INVALID_ID;
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_panel (float x, float y, float w, float h, UI_Flag flags, Vec4 c)
{
    Panel panel;

    // The method of adding a new panel varies depending on whether the panel
    // is a child to an existing panel or if it is a lone panel in the window.
    panel.absolute_bounds = { x, y, w, h };
    if (ui_panels.size() > 0)
    {
        const Quad& p_ab = ui_panels.top().absolute_bounds;
        const Quad& p_v  = ui_panels.top().viewport;
        const Vec2& p_ro = ui_panels.top().relative_offset;

        Quad& c_ab = panel.absolute_bounds;
        Quad& c_v  = panel.viewport;
        Vec2& c_ro = panel.relative_offset;

        c_ab.x += p_ab.x + p_ro.x;
        c_ab.y += p_ab.y + p_ro.y;

        c_v = c_ab;

        // We also clip the panel's viewport to be inside of the
        // parent panel to avoid issues with overlapping/spill.
        float dx = c_v.x - p_v.x;
        float dy = c_v.y - p_v.y;

        if (c_v.x < p_v.x) c_v.x = p_v.x, c_v.w -= roundf(abs(dx)), dx = c_v.x - p_v.x;
        if (c_v.y < p_v.y) c_v.y = p_v.y, c_v.h -= roundf(abs(dy)), dy = c_v.y - p_v.y;

        if (c_v.x+c_v.w > p_v.x+p_v.w) c_v.w = p_v.w - roundf(abs(dx));
        if (c_v.y+c_v.h > p_v.y+p_v.h) c_v.h = p_v.h - roundf(abs(dy));

        if (c_v.w < 0) c_v.w = 0;
        if (c_v.h < 0) c_v.h = 0;

        // And determine the panel's offset to its viewport.
        c_ro.x = c_ab.x - c_v.x;
        c_ro.y = c_ab.y - c_v.y;

        // Inherit the parent panel's flags.
        panel.flags = flags | ui_panels.top().flags;
    }
    else
    {
        panel.viewport = panel.absolute_bounds;
        panel.relative_offset = Vec2(0,0);
        panel.flags = flags;
    }

    panel.cursor = NULL;
    panel.cursor_dir = UI_DIR_RIGHT;
    panel.cursor_advance_enabled = true;

    set_viewport(panel.viewport);
    ui_panels.push(panel);

    set_draw_color(c);
    fill_quad(0, 0, panel.viewport.w, panel.viewport.h);
}

TEINAPI void begin_panel (Quad bounds, UI_Flag flags, Vec4 c)
{
    begin_panel(bounds.x, bounds.y, bounds.w, bounds.h, flags, c);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool begin_click_panel (UI_Action action, float w, float h, UI_Flag flags, std::string info)
{
    Panel& parent = ui_panels.top();

    Vec2 rcur = internal__get_relative_cursor(parent);
    Vec2 cur = internal__get_cursor(parent);

    // Cache the panel's flags so they are easily accessible.
    bool locked    = (flags&UI_LOCKED);
    bool highlight = (flags&UI_HIGHLIGHT);

    bool result = internal__handle_widget(rcur.x, rcur.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 back = ui_color_medium;

    if      (locked)             back = ui_color_med_dark;
    else if (internal__is_hit()) back = ui_color_dark;
    else if (internal__is_hot()) back = ui_color_med_light;

    begin_panel(cur.x, cur.y, w, h, flags, back);
    internal__advance_ui_cursor_start(parent, w, h);

    if (highlight && !internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        set_draw_color(color);
        fill_quad(0, 0, get_viewport().w, get_viewport().h);
    }

    Vec4 separator_color = (locked) ? ui_color_dark : ui_color_med_dark;
    Vec2 cursor = ui_panels.top().relative_offset;

    internal__draw_separator(cursor, parent.cursor_dir, w, h, separator_color);
    internal__advance_ui_cursor_end(parent, w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && internal__is_hot())
    {
        push_status_bar_message(info.c_str());
    }

    ++ui_current_id;

    return result;
}

/* -------------------------------------------------------------------------- */

TEINAPI void end_panel ()
{
    ui_panels.pop();

    // We either go back to a previous nested panel or this is the last panel
    // and we go back to placing things relative to the entire program window.
    if (ui_panels.size() > 0) set_viewport(ui_panels.top().viewport);
    else set_viewport(0, 0, get_render_target_w(), get_render_target_h());
}

/* -------------------------------------------------------------------------- */

TEINAPI float get_panel_w ()
{
    return ui_panels.top().absolute_bounds.w;
}

TEINAPI float get_panel_h ()
{
    return ui_panels.top().absolute_bounds.h;
}

/* -------------------------------------------------------------------------- */

TEINAPI Vec2 get_panel_offset ()
{
    return ui_panels.top().relative_offset;
}

TEINAPI Vec2 get_panel_cursor ()
{
    return internal__get_cursor(ui_panels.top());
}

/* -------------------------------------------------------------------------- */

TEINAPI void disable_panel_cursor_advance ()
{
    ui_panels.top().cursor_advance_enabled = false;
}

TEINAPI void enable_panel_cursor_advance ()
{
    ui_panels.top().cursor_advance_enabled = true;
}

/* -------------------------------------------------------------------------- */

TEINAPI void advance_panel_cursor (float advance)
{
    internal__advance_ui_cursor_start(ui_panels.top(), advance, advance);
    internal__advance_ui_cursor_end(ui_panels.top(), advance, advance);
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_panel_cursor (Vec2* cursor)
{
    ui_panels.top().cursor = cursor;
}

TEINAPI void set_panel_cursor_dir (UI_Dir dir)
{
    ui_panels.top().cursor_dir = dir;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_panel_flags (UI_Flag flags)
{
    ui_panels.top().flags = flags;
}

TEINAPI UI_Flag get_panel_flags ()
{
    return ui_panels.top().flags;
}

/* -------------------------------------------------------------------------- */

TEINAPI float calculate_button_txt_width (std::string text)
{
    // Important to return ceiled value otherwise the next button using the
    // cursor to position itself might overlap the previous button by 1px.
    constexpr float X_PADDING = 20;
    assert(ui_font);
    return (ceilf(get_text_width_scaled(*ui_font, text)) + X_PADDING);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool do_button_img (UI_Action action, float w, float h, UI_Flag flags, const Quad* clip, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(ui_texture);

    flags |= ui_panels.top().flags;

    bool inactive  = (flags & UI_INACTIVE);
    bool locked    = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Texture& tex = *ui_texture;
    Vec2     cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid image overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    bool result = internal__handle_widget(cur.x, cur.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front  = (ui_is_light) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 back   =  ui_color_medium;
    Vec4 shadow = (ui_is_light) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if      (locked)             back = ui_color_med_dark;
    else if (internal__is_hit()) back = ui_color_dark;
    else if (internal__is_hot()) back = ui_color_med_light;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    set_draw_color(back); // Draw the button's background quad.
    fill_quad(cur.x, cur.y, cur.x + w, cur.y + h);

    if (highlight && !internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        set_draw_color(color);
        fill_quad(0, 0, get_viewport().w, get_viewport().h);
    }

    // The ((w)-1) and ((h)-1) are used to ensure the separator does
    // not mess with the centering of the image based on direction.

    UI_Dir dir = ui_panels.top().cursor_dir;

    float w2 = (dir == UI_DIR_RIGHT || dir == UI_DIR_LEFT) ? ((w)-1) : (w);
    float h2 = (dir == UI_DIR_UP    || dir == UI_DIR_DOWN) ? ((h)-1) : (h);

    // Center the image within the button.
    float x = roundf(cur.x + (w2 / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cur.y + (h2 / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    tex.color = shadow;
    draw_texture(tex, x, y-offset, clip);
    tex.color = front;
    draw_texture(tex, x, y, clip);

    internal__draw_separator(internal__get_relative_cursor(ui_panels.top()), ui_panels.top().cursor_dir, w, h, ui_color_med_dark);
    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && internal__is_hot())
    {
        std::string kb_info;
        if (!kb.empty())
        {
            kb_info = FormatString("(%s)", get_key_binding_main_string(kb).c_str());
            if (get_key_binding(kb).alt_code && get_key_binding(kb).alt_mod)
            {
                kb_info += FormatString(" or (%s)", get_key_binding_alt_string(kb).c_str());
            }
        }
        std::string info_text((kb_info.empty()) ? info : FormatString("%s %s", kb_info.c_str(), info.c_str()));
        push_status_bar_message(info_text.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && internal__is_hot())
    {
        set_current_tooltip(name);
    }

    ++ui_current_id;

    return result;
}

TEINAPI bool do_button_txt (UI_Action action, float w, float h, UI_Flag flags, std::string text, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    flags |= ui_panels.top().flags;

    bool inactive  = (flags & UI_INACTIVE);
    bool locked    = (flags & UI_LOCKED);
    bool highlight = (flags & UI_HIGHLIGHT);
    bool single    = (flags & UI_SINGLE);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2  cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid text overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    // Locked buttons cannot be interacted with.
    bool result = internal__handle_widget(cur.x, cur.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front  = (ui_is_light) ? ui_color_black    : ui_color_ex_light;
    Vec4 back   = ui_color_medium;
    Vec4 shadow = (ui_is_light) ? ui_color_ex_light : ui_color_black;

    if      (locked)             back = ui_color_med_dark;
    else if (internal__is_hit()) back = ui_color_dark;
    else if (internal__is_hot()) back = ui_color_med_light;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    set_draw_color(back); // Draw the button's background quad.
    fill_quad(cur.x, cur.y, cur.x + w, cur.y + h);

    if (highlight && !internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        set_draw_color(color);
        fill_quad(0, 0, get_viewport().w, get_viewport().h);
    }

    float w2 = get_text_width_scaled(fnt, text);
    float h2 = fnt.line_gap.at(fnt.current_pt_size) * get_font_draw_scale();
    // Center the text within the button.
    float x = roundf(cur.x + ((w - w2) / 2));
    float y = roundf(cur.y + ((h / 2) + (h2 / 4)));

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    fnt.color = shadow;
    draw_text(fnt, x, y-offset, text);
    fnt.color = front;
    draw_text(fnt, x, y, text);

    if (!single)
    {
        internal__draw_separator(internal__get_relative_cursor(ui_panels.top()),
            ui_panels.top().cursor_dir, w, h, ui_color_med_dark);
    }

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && internal__is_hot())
    {
        std::string kb_info;
        if (!kb.empty())
        {
            kb_info = FormatString("(%s)", get_key_binding_main_string(kb).c_str());
            if (get_key_binding(kb).alt_code && get_key_binding(kb).alt_mod)
            {
                kb_info += FormatString(" or (%s)", get_key_binding_alt_string(kb).c_str());
            }
        }
        std::string info_text((kb_info.empty()) ? info : FormatString("%s %s", kb_info.c_str(), info.c_str()));
        push_status_bar_message(info_text.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && internal__is_hot())
    {
        set_current_tooltip(name);
    }

    ++ui_current_id;

    return result;
}

TEINAPI bool do_button_txt (UI_Action action, float h, UI_Flag flags, std::string text, std::string info, std::string kb, std::string name)
{
    // Important to return ceiled value otherwise the next button using the
    // cursor to position itself might overlap the previous button by 1px.
    constexpr float X_PADDING = 20;
    float w = ceilf(get_text_width_scaled(*ui_font, text)) + X_PADDING;
    return do_button_txt(action, w, h, flags, text, info, kb, name);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_label (UI_Align horz, UI_Align vert, float w, float h, std::string text, Vec4 bg)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    UI_Flag flags = ui_panels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked   = (flags & UI_LOCKED  );
    bool tooltip  = (flags & UI_TOOLTIP );
    bool darken   = (flags & UI_DARKEN  );

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2  cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid text overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    set_draw_color(bg); // Draw the label's background.
    fill_quad(cur.x, cur.y, cur.x + w, cur.y + h);

    float tw = get_text_width_scaled (fnt, text);
    float th = get_text_height_scaled(fnt, text);

    // If text is a single line we calculate how much we can fit in the width
    // and if necessary trim any off and replace the end with and ellipsis.
    bool text_clipped = false;
    std::string clipped_text(text);
    if (std::count(clipped_text.begin(), clipped_text.end(), '\n') <= 1)
    {
        if (tw > w) // Our text goes out of the label bounds.
        {
            text_clipped = true;
            if (clipped_text.length() <= 3)
            {
                clipped_text = "...";
                tw = get_text_width_scaled(fnt, clipped_text);
            }
            else
            {
                clipped_text.replace(clipped_text.length()-3, 3, "...");
                while (tw > w && clipped_text.length() > 3)
                {
                    clipped_text.erase(clipped_text.length()-4, 1);
                    tw = get_text_width_scaled(fnt, clipped_text);
                }
            }
        }
    }

    float x = cur.x;
    float y = cur.y;

    internal__align_text(horz, vert, x, y, tw, th, w, h);

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    Vec4 shadow = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 front = (ui_is_light) ? ui_color_black : ui_color_ex_light;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    if (tooltip)
    {
        shadow = UI_D_COLOR_BLACK;
        front = UI_D_COLOR_EX_LIGHT;
        if (darken)
        {
            front = Vec4(.7f,.7f,.7f, 1);
        }
    }

    fnt.color = shadow;
    draw_text(fnt, x, y-offset, clipped_text);
    fnt.color = front;
    draw_text(fnt, x, y, clipped_text);

    Quad clipped_bounds = internal__get_clipped_bounds(cur.x, cur.y, w, h);
    Vec2 mouse = GetMousePos();
    bool inside = PointInBoundsXYXY(mouse, clipped_bounds);
    if (text_clipped && inside) set_current_tooltip(text);

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);
}

TEINAPI void do_label (UI_Align horz, UI_Align vert, float h, std::string text, Vec4 bg)
{
    // Important to return ceiled value otherwise the next label using the
    // cursor to position itself might overlap the previous label by 1px.
    assert(ui_font);
    float w = ceilf(get_text_width_scaled(*ui_font, text));
    return do_label(horz, vert, w, h, text, bg);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_label_hyperlink (UI_Align horz, UI_Align vert, float w, float h, std::string text, std::string link, std::string href, Vec4 bg)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2  cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid text overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    set_draw_color(bg); // Draw the label's background.
    fill_quad(cur.x, cur.y, cur.x + w, cur.y + h);

    float tw = get_text_width_scaled (fnt, text);
    float th = get_text_height_scaled(fnt, text);

    // If text is a single line we calculate how much we can fit in the width
    // and if necessary trim any off and replace the end with and ellipsis.
    std::string clipped_text(text);
    if (std::count(clipped_text.begin(), clipped_text.end(), '\n') <= 1)
    {
        if (tw > w) // Our text goes out of the label bounds.
        {
            if (clipped_text.length() <= 3)
            {
                clipped_text = "...";
                tw = get_text_width_scaled(fnt, clipped_text);
            }
            else
            {
                clipped_text.replace(clipped_text.length()-3, 3, "...");
                while (tw > w && clipped_text.length() > 3)
                {
                    clipped_text.erase(clipped_text.length()-4, 1);
                    tw = get_text_width_scaled(fnt, clipped_text);
                }
            }
        }
    }

    // Handle setting the application's cursor to the correct graphic.
    if (internal__is_hot())
    {
        ui_hot_hyperlink = ui_current_id;
        set_cursor(Cursor::POINTER);
    }
    else
    {
        if (ui_hot_hyperlink == ui_current_id)
        {
            ui_hot_hyperlink = UI_INVALID_ID;
        }

        // We have this check so that we can know it's okay to set the cursor back to arrow as no text box elements are hot.
        if (ui_hot_hyperlink == UI_INVALID_ID && ui_hot_text_box == UI_INVALID_ID)
        {
            // NOTE: Kind of hacky to put this here, but it prevents issues with
            // the flickering of the cursor due to hyperlinks. Could be cleaned.
            if (current_tab_is_level() && mouse_inside_level_editor_viewport() && IsWindowFocused("WINMAIN"))
            {
                switch (level_editor.tool_type)
                {
                case (Tool_Type::BRUSH ): set_cursor(Cursor::BRUSH);  break;
                case (Tool_Type::FILL  ): set_cursor(Cursor::FILL);   break;
                case (Tool_Type::SELECT): set_cursor(Cursor::SELECT); break;
                }
            }
            else
            {
                set_cursor(Cursor::ARROW);
            }
        }
    }

    float wx = cur.x + get_text_width_scaled(fnt, clipped_text);
    float wy = cur.y;
    float ww = get_text_width_scaled (fnt, link);
    float wh = get_text_height_scaled(fnt, link);

    if (internal__handle_widget(wx,wy,ww,wh, false)) LoadWebpage(href);

    float x = cur.x;
    float y = cur.y;

    internal__align_text(horz, vert, x, y, tw, th, w, h);

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    Vec4 shadow     = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 front      = (ui_is_light) ? ui_color_black : ui_color_ex_light;
    Vec4 link_color = front;

    if      (internal__is_hit()) link_color = (ui_is_light) ? ui_color_ex_dark : ui_color_white;
    else if (internal__is_hot()) link_color = (ui_is_light) ? ui_color_ex_dark : ui_color_white;

    fnt.color = shadow;
    draw_text(fnt, x, y-offset, clipped_text);
    fnt.color = front;
    draw_text(fnt, x, y, clipped_text);

    x += get_text_width_scaled(fnt, clipped_text);

    set_draw_color(shadow);
    draw_line(x, (y+2)-offset, x+ww, (y+2)-offset);
    set_draw_color(link_color);
    draw_line(x, y+2, x+ww, y+2);

    fnt.color = shadow;
    draw_text(fnt, x, y-offset, link);
    fnt.color = link_color;
    draw_text(fnt, x, y, link);

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    ++ui_current_id;
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_markdown (float w, float h, std::string text)
{
    Font& fnt = get_editor_regular_font();

    UI_Flag flags = ui_panels.top().flags;

    bool inactive = (flags & UI_INACTIVE);
    bool locked   = (flags & UI_LOCKED  );

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid text overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    std::vector<std::string> lines;
    TokenizeString(text, "\r\n", lines);

    float x = cur.x;
    float y = cur.y + fnt.line_gap[fnt.current_pt_size];

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    Vec4 shadow = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 front = (ui_is_light) ? ui_color_black : ui_color_ex_light;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    internal__do_markdown_formatting(lines, w);

    for (auto& line: lines)
    {
        std::vector<std::string> sub_lines;
        TokenizeString(line, "\r\n", sub_lines);

        for (size_t i=0; i<sub_lines.size(); ++i)
        {
            x = cur.x;
            if (i != 0) x += get_text_width_scaled(fnt, ">");
            fnt.color = shadow;
            draw_text(fnt, x, y-offset, sub_lines.at(i));
            fnt.color = front;
            draw_text(fnt, x, y, sub_lines.at(i));
            y += fnt.line_gap[fnt.current_pt_size];
        }
    }

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);
}

TEINAPI float get_markdown_h (float w, std::string text)
{
    Font& fnt = get_editor_regular_font();

    std::vector<std::string> lines;
    TokenizeString(text, "\r\n", lines);

    std::string md_text = internal__do_markdown_formatting(lines, w);

    return get_text_height_scaled(fnt, md_text);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_text_box (float w, float h, UI_Flag flags, std::string& text, std::string default_text, UI_Align halign)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    flags |= ui_panels.top().flags;
    bool locked = (flags & UI_LOCKED);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    if (!locked)
    {
        if (internal__handle_widget(cur.x, cur.y, w, h, locked)) {
            // If the cursor was blinking before then reset the timer.
            if (ui_cursor_blink_timer)
            {
                SDL_RemoveTimer(ui_cursor_blink_timer);
                ui_cursor_blink_timer = NULL;
            }

            // Start the blinking of the cursor.
            ui_cursor_visible = true;
            ui_cursor_blink_timer = SDL_AddTimer(UI_CURSOR_BLINK_INTERVAL, internal__cursor_blink_callback, NULL);
            if (!ui_cursor_blink_timer)
            {
                LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
            }

            ui_text_box_cursor = text.length();
            ui_active_text_box = ui_current_id;

            SDL_StartTextInput();
        }
    }
    else if (ui_active_text_box == ui_current_id)
    {
        deselect_active_text_box(text, default_text);
    }

    // If we are the active text box and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if (ui_active_text_box == ui_current_id && internal__is_ui_mouse_down() && !internal__is_hit())
    {
        deselect_active_text_box(text, default_text);
    }
    // If the right mouse button is pressed then we just always deselect.
    if (ui_active_text_box == ui_current_id && internal__is_ui_mouse_r_down())
    {
        deselect_active_text_box(text, default_text);
    }

    // Handle setting the application's cursor to the correct graphic.
    if (internal__is_hot())
    {
        ui_hot_text_box = ui_current_id;
        set_cursor(Cursor::BEAM);
    }
    else
    {
        if (ui_hot_text_box == ui_current_id)
        {
            ui_hot_text_box = UI_INVALID_ID;
        }

        // We have this check so that we can know it's okay to set
        // the cursor back to arrow as no text box elements are hot.
        if (ui_hot_text_box == UI_INVALID_ID && ui_hot_hyperlink == UI_INVALID_ID)
        {
            // NOTE: Kind of hacky to put this here, but it prevents issues with
            // the flickering of the cursor due to text boxes. Could be cleaned.
            if (current_tab_is_level() && mouse_inside_level_editor_viewport() && IsWindowFocused("WINMAIN"))
            {
                switch (level_editor.tool_type)
                {
                case (Tool_Type::BRUSH ): set_cursor(Cursor::BRUSH ); break;
                case (Tool_Type::FILL  ): set_cursor(Cursor::FILL  ); break;
                case (Tool_Type::SELECT): set_cursor(Cursor::SELECT); break;
                }
            }
            else
            {
                set_cursor(Cursor::ARROW);
            }
        }
    }

    if (!locked && ui_make_next_text_box_active && get_render_target()->id == ui_text_box_tab_window_id)
    {
        ui_text_box_cursor = std::string::npos;
        ui_active_text_box = ui_current_id;

        ui_make_next_text_box_active = false;
        ui_text_box_tab_window_id = 0;
    }

    Vec4 front   = (ui_is_light) ? ui_color_black : ui_color_ex_light;
    Vec4 shadow  = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 outline = ui_color_dark;
    Vec4 back    = ui_color_med_dark;

    if (locked)
    {
        outline = ui_color_med_dark;
        back = ui_color_medium;

        shadow.a = .5f;
        front.a = .5f;
    }

    set_draw_color(outline); // Draw the text box's outline quad.
    fill_quad(cur.x, cur.y, cur.x+w, cur.y+h);
    set_draw_color(back); // Draw the text box's background quad.
    fill_quad(cur.x+1, cur.y+1, cur.x+w-1, cur.y+h-1);

    constexpr float X_PAD = 5;
    constexpr float Y_PAD = 2;

    float bx = cur.x+(X_PAD  );
    float by = cur.y+(Y_PAD  );
    float bw = w    -(X_PAD*2);
    float bh = h    -(Y_PAD*2);

    // Handle text input events if we are the active text box.
    if (ui_active_text_box == ui_current_id)
    {
        // Make sure that the cursor is in the bounds of the string.
        if (ui_text_box_cursor > text.length())
        {
            ui_text_box_cursor = text.length();
        }

        if (get_render_target()->focus)
        {
            std::string old_text = text;
            size_t old_cursor = ui_text_box_cursor;

            for (auto& text_event: ui_text_events)
            {
                switch (text_event.type)
                {
                    case (UI_Text_Event_Type::TEXT):
                    {
                        bool invalid_text = false;
                        for (auto c: text_event.text)
                        {
                            if ((flags&UI_ALPHANUM)   && !isalnum               (c)) invalid_text = true;
                            if ((flags&UI_ALPHABETIC) && !isalpha               (c)) invalid_text = true;
                            if ((flags&UI_NUMERIC)    && !isdigit               (c)) invalid_text = true;
                            if ((flags&UI_FILEPATH)   && !internal__is_file_path(c)) invalid_text = true;
                        }
                        if (invalid_text)
                        {
                            break;
                        }
                        // Clear out the default text and enter what the user actually wants.
                        if (!default_text.empty() && (text == default_text))
                        {
                            ui_text_box_cursor = 0;
                            text.clear();
                        }
                        for (auto c: text_event.text)
                        {
                            auto pos = text.begin()+(ui_text_box_cursor++);
                            text.insert(pos, c);
                        }
                    } break;
                    case (UI_Text_Event_Type::KEY):
                    {
                        switch (text_event.key)
                        {
                            case (SDLK_TAB):
                            {
                                if (!ui_tab_handled)
                                {
                                    ui_make_next_text_box_active = true;
                                    ui_text_box_tab_window_id = get_render_target()->id;
                                    ui_tab_handled = true;
                                }
                            } break;
                            case (SDLK_LEFT):
                            {
                                if (ui_text_box_cursor > 0)
                                {
                                    --ui_text_box_cursor;
                                }
                            } break;
                            case (SDLK_RIGHT): {
                                if (ui_text_box_cursor < text.length())
                                {
                                    ++ui_text_box_cursor;
                                }
                            } break;
                            case (SDLK_UP):
                            {
                                if (flags&UI_NUMERIC)
                                {
                                    if (atoi(text.c_str()) < INT_MAX)
                                    {
                                        text = std::to_string(atoi(text.c_str())+1);
                                        ui_text_box_cursor = text.length();
                                    }
                                }
                            } break;
                            case (SDLK_DOWN):
                            {
                                if (flags&UI_NUMERIC)
                                {
                                    if (atoi(text.c_str()) > 0)
                                    {
                                        text = std::to_string(atoi(text.c_str())-1);
                                        ui_text_box_cursor = text.length();
                                    }
                                }
                            } break;
                            case (SDLK_HOME):
                            {
                                ui_text_box_cursor = 0;
                            } break;
                            case (SDLK_END):
                            {
                                ui_text_box_cursor = text.length();
                            } break;
                            case (SDLK_BACKSPACE):
                            {
                                if (ui_text_box_cursor != 0)
                                {
                                    text.erase(--ui_text_box_cursor, 1);
                                }
                            } break;
                            case (SDLK_DELETE):
                            {
                                if (ui_text_box_cursor < text.length())
                                {
                                    text.erase(ui_text_box_cursor, 1);
                                }
                            } break;
                            case (SDLK_RETURN):
                            {
                                deselect_active_text_box(text, default_text);
                            } break;
                            case (SDLK_v):
                            {
                                if (SDL_GetModState()&KMOD_CTRL)
                                {
                                    if (SDL_HasClipboardText())
                                    {
                                        char* clipboard_text = SDL_GetClipboardText();
                                        if (clipboard_text)
                                        {
                                            Defer { SDL_free(clipboard_text); }; // Docs say we need to free!

                                            bool add_text = true;
                                            std::string t(clipboard_text);

                                            for (auto c: t)
                                            {
                                                if ((flags&UI_ALPHANUM)   && !isalnum               (c)) { add_text = false; break; }
                                                if ((flags&UI_ALPHABETIC) && !isalpha               (c)) { add_text = false; break; }
                                                if ((flags&UI_NUMERIC)    && !isdigit               (c)) { add_text = false; break; }
                                                if ((flags&UI_FILEPATH)   && !internal__is_file_path(c)) { add_text = false; break; }
                                            }

                                            if (add_text)
                                            {
                                                // Clear out the default text and enter what the user actually wants.
                                                if (!default_text.empty() && text == default_text)
                                                {
                                                    ui_text_box_cursor = 0;
                                                    text.clear();
                                                }

                                                text.insert(ui_text_box_cursor, t);
                                                ui_text_box_cursor += t.length();
                                            }
                                        }
                                    }
                                }
                            } break;
                        }
                    } break;
                }
            }

            ui_text_events.clear();

            // Reset the cursor blink interval.
            if (old_text != text || old_cursor != ui_text_box_cursor)
            {
                // If the cursor was blinking before then reset the timer.
                if (ui_cursor_blink_timer)
                {
                    SDL_RemoveTimer(ui_cursor_blink_timer);
                    ui_cursor_blink_timer = NULL;
                }
                // Start the blinking of the cursor.
                ui_cursor_visible = true;
                ui_cursor_blink_timer = SDL_AddTimer(UI_CURSOR_BLINK_INTERVAL, internal__cursor_blink_callback, NULL);
                if (!ui_cursor_blink_timer)
                {
                    LogError(ERR_MIN, "Failed to setup cursor blink timer! (%s)", SDL_GetError());
                }
            }
        }

        // Cursor should always be at the end of the default text.
        if (!default_text.empty() && text == default_text)
        {
            ui_text_box_cursor = text.length();
        }
    }

    // We scissor the contents to avoid text overspill.
    begin_scissor(bx, by, bw, bh);

    float tx = bx;
    float ty = by;
    float tw = get_text_width_scaled (fnt, text);
    float th = get_text_height_scaled(fnt, text);

    if (th == 0) th = bh;

    internal__align_text(halign, UI_ALIGN_CENTER, tx, ty, tw, th, bw, bh);

    float x_off = 0;
    float y_off = (ui_is_light) ? -1.0f : 1.0f;

    // Adjust text position/offsetrun based on the current cursor.
    if (ui_active_text_box == ui_current_id)
    {
        if (halign == UI_ALIGN_LEFT)
        {
            std::string sub(text.substr(0, ui_text_box_cursor));
            float cursor_x = tx+get_text_width_scaled(fnt, sub);
            if (cursor_x > bx+bw)
            {
                float diff = abs(bw - get_text_width_scaled(fnt, sub));
                x_off = -diff;
            }
        }
        else
        {
            std::string sub(text.substr(0, ui_text_box_cursor));
            float cursor_x = tx+get_text_width_scaled(fnt, sub);
            if (cursor_x < bx)
            {
                x_off = (bx - cursor_x);
            }
        }
    }

    fnt.color = shadow;
    draw_text(fnt, tx+x_off, ty-y_off, text);
    fnt.color = front;
    draw_text(fnt, tx+x_off, ty, text);

    end_scissor();

    // If we're active then draw the text box cursor as well.
    if (ui_active_text_box == ui_current_id && ui_cursor_visible)
    {
        begin_scissor(bx-1, by-1, bw+2, bh+2);

        std::string sub(text.substr(0, ui_text_box_cursor));
        float xo = get_text_width_scaled(fnt, sub);
        float yo = (bh-th)/2; // Center the cursor vertically.
        // Just looks nicer...
        if ((ui_text_box_cursor != 0 && text.length()) || (!text.length()))
        {
            xo += 1;
        }
        set_draw_color((ui_is_light) ? ui_color_black : ui_color_ex_light);
        draw_line(tx+xo+x_off, by+yo, tx+xo+x_off, by+yo+th);

        end_scissor();
    }

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    ++ui_current_id;
}

TEINAPI void do_text_box_labeled (float w, float h, UI_Flag flags, std::string& text, float label_w, std::string label, std::string default_text, UI_Align halign)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    float lw = label_w;
    float tw = w - lw;

    if (tw < 0) return; // Won't draw anything!

    // Cache this stuff because we are going to temporarily change it.
    Vec2 cursor = *ui_panels.top().cursor;
    UI_Dir dir = ui_panels.top().cursor_dir;

    set_panel_cursor_dir(UI_DIR_RIGHT);
    do_label(UI_ALIGN_LEFT, UI_ALIGN_CENTER, lw, h, label);

    set_panel_cursor_dir(dir);
    do_text_box(tw, h, flags, text, default_text, halign);

    // Reset the X location of the cursor for the caller.
    if (dir == UI_DIR_UP || dir == UI_DIR_DOWN)
    {
        ui_panels.top().cursor->x = cursor.x;
    }
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_hotkey_rebind_main (float w, float h, UI_Flag flags, Key_Binding& kb)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    flags |= ui_panels.top().flags;
    bool locked = (flags & UI_LOCKED);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    if (!locked)
    {
        if (internal__handle_widget(cur.x, cur.y, w, h, locked))
        {
            ui_active_hotkey_rebind = ui_current_id;
        }
    }
    else if (ui_active_hotkey_rebind == ui_current_id)
    {
        deselect_active_hotkey_rebind();
    }

    // If we are the active KB rebind and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if (ui_active_hotkey_rebind == ui_current_id && internal__is_ui_mouse_down() && !internal__is_hit())
    {
        deselect_active_hotkey_rebind();
    }
    // If the right mouse button is pressed then we just always deselect.
    if (ui_active_hotkey_rebind == ui_current_id && internal__is_ui_mouse_r_down())
    {
        deselect_active_hotkey_rebind();
    }

    Vec4 front   = (ui_is_light) ? ui_color_black : ui_color_ex_light;
    Vec4 shadow  = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 outline = ui_color_dark;
    Vec4 back    = ui_color_med_dark;

    if (locked)
    {
        outline = ui_color_med_dark;
        back = ui_color_medium;

        shadow.a = .5f;
        front.a = .5f;
    }

    set_draw_color(outline); // Draw the rebind's outline quad.
    fill_quad(cur.x, cur.y, cur.x+w, cur.y+h);
    set_draw_color(back); // Draw the rebind's background quad.
    fill_quad(cur.x+1, cur.y+1, cur.x+w-1, cur.y+h-1);

    constexpr float X_PAD = 5;
    constexpr float Y_PAD = 2;

    float bx = cur.x+(X_PAD  );
    float by = cur.y+(Y_PAD  );
    float bw = w    -(X_PAD*2);
    float bh = h    -(Y_PAD*2);

    // If we're active then we check if the user has entered a new binding.
    if (ui_active_hotkey_rebind == ui_current_id)
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            SDL_Keycode k = main_event.key.keysym.sym;
            // We do not want the key binding to be set when just the mod is pressed!
            if (k != SDLK_LCTRL  && k != SDLK_RCTRL  &&
                k != SDLK_LALT   && k != SDLK_RALT   &&
                k != SDLK_MODE                       &&
                k != SDLK_RSHIFT && k != SDLK_LSHIFT &&
                k != SDLK_LGUI   && k != SDLK_RGUI)
            {
                kb.code = main_event.key.keysym.sym;

                // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
                kb.mod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));

                // We do not care whether the right or left variants have been pressed.
                if (kb.mod&KMOD_LCTRL  || kb.mod&KMOD_RCTRL)  kb.mod |= KMOD_CTRL;
                if (kb.mod&KMOD_LALT   || kb.mod&KMOD_RALT)   kb.mod |= KMOD_ALT;
                if (kb.mod&KMOD_LSHIFT || kb.mod&KMOD_RSHIFT) kb.mod |= KMOD_SHIFT;
                if (kb.mod&KMOD_LGUI   || kb.mod&KMOD_RGUI)   kb.mod |= KMOD_GUI;

                deselect_active_hotkey_rebind();
            }
        }
    }

    // We scissor the contents to avoid text overspill.
    begin_scissor(bx, by, bw, bh);

    // The text to display depends on if we're active or not.
    std::string text;
    if (ui_active_hotkey_rebind == ui_current_id)
    {
        text = "Enter new key binding...";
    }
    else
    {
        text = get_key_binding_main_string(kb);
    }

    // Calculate the position of the text and draw it
    float tx = bx;
    float ty = by;
    float tw = get_text_width_scaled (fnt, text);
    float th = get_text_height_scaled(fnt, text);

    internal__align_text(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, tx, ty, tw, th, bw, bh);

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    fnt.color = shadow;
    draw_text(fnt, tx, ty-offset, text);
    fnt.color = front;
    draw_text(fnt, tx, ty, text);

    end_scissor();

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    ++ui_current_id;
}

TEINAPI void do_hotkey_rebind_alt (float w, float h, UI_Flag flags, Key_Binding& kb)
{
    // Make sure that the necessary components are assigned.
    assert(ui_font);

    // Cache the rebind's flags so they are easily accessible.
    flags |= ui_panels.top().flags;
    bool locked = (flags & UI_LOCKED);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Font& fnt = *ui_font;
    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    if (!locked)
    {
        if (internal__handle_widget(cur.x, cur.y, w, h, locked))
        {
            ui_active_hotkey_rebind = ui_current_id;
        }
    }
    else if (ui_active_hotkey_rebind == ui_current_id)
    {
        deselect_active_hotkey_rebind();
    }

    // If we are the active KB rebind and the mouse was pressed this update
    // and we're not hit then that means the press was outside of us and
    // therefore we need to become deselected and can no longer be active.
    if (ui_active_hotkey_rebind == ui_current_id && internal__is_ui_mouse_down() && !internal__is_hit())
    {
        deselect_active_hotkey_rebind();
    }
    // If the right mouse button is pressed then we just always deselect.
    if (ui_active_hotkey_rebind == ui_current_id && internal__is_ui_mouse_r_down())
    {
        deselect_active_hotkey_rebind();
    }

    Vec4 front   = (ui_is_light) ? ui_color_black : ui_color_ex_light;
    Vec4 shadow  = (ui_is_light) ? ui_color_ex_light : ui_color_black;
    Vec4 outline = ui_color_dark;
    Vec4 back    = ui_color_med_dark;

    if (locked)
    {
        outline = ui_color_med_dark;
        back = ui_color_medium;

        shadow.a = .5f;
        front.a = .5f;
    }

    set_draw_color(outline); // Draw the rebind's outline quad.
    fill_quad(cur.x, cur.y, cur.x+w, cur.y+h);
    set_draw_color(back); // Draw the rebind's background quad.
    fill_quad(cur.x+1, cur.y+1, cur.x+w-1, cur.y+h-1);

    constexpr float X_PAD = 5;
    constexpr float Y_PAD = 2;

    float bx = cur.x+(X_PAD  );
    float by = cur.y+(Y_PAD  );
    float bw = w    -(X_PAD*2);
    float bh = h    -(Y_PAD*2);

    // If we're active then we check if the user has entered a new binding.
    if (ui_active_hotkey_rebind == ui_current_id)
    {
        if (main_event.type == SDL_KEYDOWN)
        {
            SDL_Keycode k = main_event.key.keysym.sym;
            // We do not want the key binding to be set when just the mod is pressed!
            if (k != SDLK_LCTRL  && k != SDLK_RCTRL  &&
                k != SDLK_LALT   && k != SDLK_RALT   &&
                k != SDLK_MODE                       &&
                k != SDLK_RSHIFT && k != SDLK_LSHIFT &&
                k != SDLK_LGUI   && k != SDLK_RGUI)
            {
                kb.alt_code = main_event.key.keysym.sym;

                // Remove CAPSLOCK and NUMLOCK because we don't care about them at all.
                kb.alt_mod = (SDL_GetModState() & ~(KMOD_NUM|KMOD_CAPS));

                // We do not care whether the right or left variants have been pressed.
                if (kb.alt_mod&KMOD_LCTRL  || kb.alt_mod&KMOD_RCTRL)  kb.alt_mod |= KMOD_CTRL;
                if (kb.alt_mod&KMOD_LALT   || kb.alt_mod&KMOD_RALT)   kb.alt_mod |= KMOD_ALT;
                if (kb.alt_mod&KMOD_LSHIFT || kb.alt_mod&KMOD_RSHIFT) kb.alt_mod |= KMOD_SHIFT;
                if (kb.alt_mod&KMOD_LGUI   || kb.alt_mod&KMOD_RGUI)   kb.alt_mod |= KMOD_GUI;

                deselect_active_hotkey_rebind();

                kb.has_alt = true;
            }
        }
    }

    // We scissor the contents to avoid text overspill.
    begin_scissor(bx, by, bw, bh);

    // The text to display depends on if we're active or not.
    std::string text;
    if (ui_active_hotkey_rebind == ui_current_id)
    {
        text = "Enter new key binding...";
    }
    else
    {
        text = get_key_binding_alt_string(kb);
    }

    // Calculate the position of the text and draw it
    float tx = bx;
    float ty = by;
    float tw = get_text_width_scaled (fnt, text);
    float th = get_text_height_scaled(fnt, text);

    internal__align_text(UI_ALIGN_RIGHT, UI_ALIGN_CENTER, tx, ty, tw, th, bw, bh);

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    fnt.color = shadow;
    draw_text(fnt, tx, ty-offset, text);
    fnt.color = front;
    draw_text(fnt, tx, ty, text);

    end_scissor();

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    ++ui_current_id;
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_icon (float w, float h, Texture& tex, const Quad* clip)
{
    UI_ID flags = ui_panels.top().flags;

    bool inactive = (flags&UI_INACTIVE);
    bool locked   = (flags&UI_LOCKED);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid image overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    Vec4 front  = (ui_is_light) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 shadow = (ui_is_light) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    UI_Dir dir = ui_panels.top().cursor_dir;

    // Center the image within the space.
    float x = roundf(cur.x + (w / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cur.y + (h / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    tex.color = shadow;
    draw_texture(tex, x, y-offset, clip);
    tex.color = front;
    draw_texture(tex, x, y, clip);

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);
}

TEINAPI void do_quad (float w, float h, Vec4 color)
{
    UI_ID flags = ui_panels.top().flags;

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Vec2 cur = internal__get_relative_cursor(ui_panels.top());

    bool inactive = (flags&UI_INACTIVE);
    bool locked   = (flags&UI_LOCKED);

    if (locked || inactive) color.a = .5f;

    set_draw_color(color);
    fill_quad(cur.x, cur.y, cur.x+w, cur.y+h);

    internal__advance_ui_cursor_end(ui_panels.top(), w, h);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_separator (float size)
{
    float w = (ui_panels.top().cursor_dir == UI_DIR_RIGHT || ui_panels.top().cursor_dir == UI_DIR_LEFT) ? 0 : size;
    float h = (ui_panels.top().cursor_dir == UI_DIR_UP    || ui_panels.top().cursor_dir == UI_DIR_DOWN) ? 0 : size;

    internal__advance_ui_cursor_start(ui_panels.top(), 1, 1);
    internal__draw_separator(internal__get_relative_cursor(ui_panels.top()), ui_panels.top().cursor_dir, w, h, ui_color_med_dark);
    internal__advance_ui_cursor_end(ui_panels.top(), 1, 1);
}

/* -------------------------------------------------------------------------- */

TEINAPI void do_scrollbar (Quad bounds, float content_height, float& scroll_offset)
{
    do_scrollbar(bounds.x, bounds.y, bounds.w, bounds.h, content_height, scroll_offset);
}

TEINAPI void do_scrollbar (float x, float y, float w, float h, float content_height, float& scroll_offset)
{
    // Allows scrollbars to be outside the panel they are scrolling.
    set_viewport(0, 0, get_render_target_w(), get_render_target_h());
    Defer { set_viewport(ui_panels.top().viewport); };

    x += ui_panels.top().viewport.x;
    y += ui_panels.top().viewport.y;

    // We scissor the contents to avoid any overspill.
    begin_scissor(x, y, w, h);
    Defer { end_scissor(); };

    constexpr float PAD = 1;

    float bx = x +  PAD;
    float by = y +  PAD;
    float bw = w - (PAD*2);
    float bh = h;

    // Determine the percentage of content visible.
    float percent_visible = get_panel_h() / content_height;
    if (percent_visible > 1) percent_visible = 1;

    // Represent this amount in the scrollbar.
    bh = (bh*percent_visible) - (PAD*2);

    // Ensure the normalized scroll offset is in bounds.
    float ndc_h = (h-bh-(PAD*2)) / (h-(PAD*2));
    scroll_offset = std::clamp(scroll_offset, 0.0f, ndc_h);

    // Convert the normalized scroll offset into pixel offset.
    by += scroll_offset * (h-(PAD*2));

    internal__handle_widget(bx, by, bw, bh, false);

    // Adjust the offset by however much the mouse has moved.
    if (internal__is_hit())
    {
        scroll_offset += (ui_mouse_relative.y / (h-(PAD*2)));
        scroll_offset = std::clamp(scroll_offset, 0.0f, ndc_h);
    }

    set_draw_color(ui_color_ex_dark);
    fill_quad(x, y, x+w, y+h);

    Vec4 color = ui_color_med_dark;

    if      (internal__is_hit()) color = ui_color_med_light;
    else if (internal__is_hot()) color = ui_color_medium;

    set_draw_color(color);
    fill_quad(bx, by, bx+bw, by+bh);

    // Draw the three lines on the scrollbar (avoid overspill).
    //
    // We do not bother though if it's really small because then
    // adding these lines just looks sort of gross and cluttered.
    if (bh > 10)
    {
        constexpr float LINE_GAP = 2;

        begin_scissor(bx, by, bw, bh);
        set_draw_color(ui_color_ex_dark);

        float y1 = by+(bh/2) - LINE_GAP;
        float y2 = by+(bh/2);
        float y3 = by+(bh/2) + LINE_GAP;

        draw_line(bx+1, y1, bx+bw-1, y1);
        draw_line(bx+1, y2, bx+bw-1, y2);
        draw_line(bx+1, y3, bx+bw-1, y3);

        end_scissor();
    }

    // Set the panels relative offset to apply the actual scroll.
    //
    // We do this extra check stuff because towards the end of
    // scrolling there can be offset issues which result in
    // moving past the content bounds so we just perform manual
    // adjustment of the scroll in order to correct this issue.
    float final_offset = roundf(content_height * scroll_offset);
    float resulting_height = content_height - final_offset;
    if (scroll_offset != 0 && resulting_height < get_panel_h())
    {
        float difference = get_panel_h() - resulting_height;
        ui_panels.top().relative_offset.y -= (final_offset - difference);
    }
    else
    {
        ui_panels.top().relative_offset.y -= roundf(content_height * scroll_offset);
    }

    ++ui_current_id;
}

/* -------------------------------------------------------------------------- */

TEINAPI void begin_panel_gradient (float x, float y, float w, float h, UI_Flag flags, Vec4 cl, Vec4 cr)
{
    Panel panel;

    // The method of adding a new panel varies depending on whether the panel
    // is a child to an existing panel or if it is a lone panel in the window.
    panel.absolute_bounds = { x, y, w, h };
    if (ui_panels.size() > 0)
    {
        const Quad& p_ab = ui_panels.top().absolute_bounds;
        const Quad& p_v  = ui_panels.top().viewport;
        const Vec2& p_ro = ui_panels.top().relative_offset;

        Quad& c_ab = panel.absolute_bounds;
        Quad& c_v  = panel.viewport;
        Vec2& c_ro = panel.relative_offset;

        c_ab.x += p_ab.x + p_ro.x;
        c_ab.y += p_ab.y + p_ro.y;

        c_v = c_ab;

        // We also clip the panel's viewport to be inside of the
        // parent panel to avoid issues with overlapping/spill.
        float dx = c_v.x - p_v.x;
        float dy = c_v.y - p_v.y;

        if (c_v.x < p_v.x) c_v.x = p_v.x, c_v.w -= roundf(abs(dx)), dx = c_v.x - p_v.x;
        if (c_v.y < p_v.y) c_v.y = p_v.y, c_v.h -= roundf(abs(dy)), dy = c_v.y - p_v.y;

        if (c_v.x+c_v.w > p_v.x+p_v.w) c_v.w = p_v.w - roundf(abs(dx));
        if (c_v.y+c_v.h > p_v.y+p_v.h) c_v.h = p_v.h - roundf(abs(dy));

        if (c_v.w < 0) c_v.w = 0;
        if (c_v.h < 0) c_v.h = 0;

        // And determine the panel's offset to its viewport.
        c_ro.x = c_ab.x - c_v.x;
        c_ro.y = c_ab.y - c_v.y;

        // Inherit the parent panel's flags.
        panel.flags = flags | ui_panels.top().flags;
    }
    else
    {
        panel.viewport = panel.absolute_bounds;
        panel.relative_offset = Vec2(0,0);
        panel.flags = flags;
    }

    panel.cursor = NULL;
    panel.cursor_dir = UI_DIR_RIGHT;
    panel.cursor_advance_enabled = true;

    set_viewport(panel.viewport);
    ui_panels.push(panel);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(0,                panel.viewport.h, cl); // BL
    put_vertex(0,                0,                cl); // TL
    put_vertex(panel.viewport.w, panel.viewport.h, cr); // BR
    put_vertex(panel.viewport.w, 0,                cr); // TR
    end_draw();
}

TEINAPI void begin_panel_gradient (Quad bounds, UI_Flag flags, Vec4 cl, Vec4 cr)
{
    begin_panel_gradient(bounds.x, bounds.y, bounds.w, bounds.h, flags, cl, cr);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool begin_click_panel_gradient (UI_Action action, float w, float h, UI_Flag flags, std::string info)
{
    Panel& parent = ui_panels.top();

    Vec2 rcur = internal__get_relative_cursor(parent);
    Vec2 cur = internal__get_cursor(parent);

    bool locked    = (flags&UI_LOCKED);
    bool highlight = (flags&UI_HIGHLIGHT);

    bool result = internal__handle_widget(rcur.x, rcur.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 bl = ui_color_medium;
    Vec4 br = ui_color_medium;

    if      (locked)             bl = ui_color_med_dark, br = ui_color_med_dark;
    else if (internal__is_hit()) bl = ui_color_dark;
    else if (internal__is_hot()) bl = ui_color_med_light;

    begin_panel_gradient(cur.x, cur.y, w, h, flags, bl, br);
    internal__advance_ui_cursor_start(parent, w, h);

    if (highlight && !internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        set_draw_color(color);
        fill_quad(0, 0, get_viewport().w, get_viewport().h);
    }
    if (highlight && internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        begin_draw(Buffer_Mode::TRIANGLE_STRIP);
        put_vertex(0,                get_viewport().h, Vec4(0,0,0,0)); // BL
        put_vertex(0,                0   ,             Vec4(0,0,0,0)); // TL
        put_vertex(get_viewport().w, get_viewport().h,         color); // BR
        put_vertex(get_viewport().w, 0,                        color); // TR
        end_draw();
    }

    Vec4 separator_color = (locked) ? ui_color_dark : ui_color_med_dark;
    Vec2 cursor = ui_panels.top().relative_offset;

    internal__draw_separator(cursor, parent.cursor_dir, w, h, separator_color);
    internal__advance_ui_cursor_end(parent, w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && internal__is_hot())
    {
        push_status_bar_message(info.c_str());
    }

    ++ui_current_id;

    return result;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool do_button_img_gradient (UI_Action action, float w, float h, UI_Flag flags, const Quad* clip, std::string info, std::string kb, std::string name)
{
    // Make sure that the necessary components are assigned.
    assert(ui_texture);

    flags |= ui_panels.top().flags;

    bool inactive  = (flags&UI_INACTIVE);
    bool locked    = (flags&UI_LOCKED);
    bool highlight = (flags&UI_HIGHLIGHT);

    internal__advance_ui_cursor_start(ui_panels.top(), w, h);

    Texture& tex = *ui_texture;
    Vec2     cur = internal__get_relative_cursor(ui_panels.top());

    // We scissor the contents to avoid image overspill.
    begin_scissor(cur.x, cur.y, w, h);
    Defer { end_scissor(); };

    // Locked buttons cannot be interacted with.
    bool result = internal__handle_widget(cur.x, cur.y, w, h, locked);
    if (result && action) action(); // Make sure action is valid!

    Vec4 front  = (ui_is_light) ? Vec4(.4f,.4f,.4f, 1) : Vec4(.73f,.73f,.73f, 1);
    Vec4 bl     =  ui_color_medium;
    Vec4 br     =  ui_color_medium;
    Vec4 shadow = (ui_is_light) ? Vec4(.9f,.9f,.9f, 1) : Vec4(.16f,.16f,.16f, 1);

    if      (locked)             br = ui_color_med_dark, bl = ui_color_med_dark;
    else if (internal__is_hit()) br = ui_color_dark;
    else if (internal__is_hot()) br = ui_color_med_light;

    if (locked || inactive)
    {
        shadow.a = .5f;
        front.a = .5f;
    }

    // Draw the button's background quad.
    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(cur.x,   cur.y+h, bl); // BL
    put_vertex(cur.x,   cur.y,   bl); // TL
    put_vertex(cur.x+w, cur.y+h, br); // BR
    put_vertex(cur.x+w, cur.y,   br); // TR
    end_draw();

    if (highlight && !internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        set_draw_color(color);
        fill_quad(0, 0, get_viewport().w, get_viewport().h);
    }
    if (highlight && internal__is_hit())
    {
        Vec4 color = ui_color_med_light;
        color.a = .66f;
        begin_draw(Buffer_Mode::TRIANGLE_STRIP);
        put_vertex(cur.x,   cur.y+h,         color); // BL
        put_vertex(cur.x,   cur.y,           color); // TL
        put_vertex(cur.x+w, cur.y+h, Vec4(0,0,0,0)); // BR
        put_vertex(cur.x+w, cur.y,   Vec4(0,0,0,0)); // TR
        end_draw();
    }

    // The ((w)-1) and ((h)-1) are used to ensure the separator does
    // not mess with the centering of the image based on direction.

    UI_Dir dir = ui_panels.top().cursor_dir;

    float w2 = (dir == UI_DIR_RIGHT || dir == UI_DIR_LEFT) ? ((w)-1) : (w);
    float h2 = (dir == UI_DIR_UP    || dir == UI_DIR_DOWN) ? ((h)-1) : (h);

    // Center the image within the button.
    float x = roundf(cur.x + (w2 / 2) + ((dir == UI_DIR_LEFT) ? 1 : 0));
    float y = roundf(cur.y + (h2 / 2) + ((dir == UI_DIR_UP)   ? 1 : 0));

    float offset = (ui_is_light) ? -1.0f : 1.0f;

    tex.color = shadow;
    draw_texture(tex, x, y-offset, clip);
    tex.color = front;
    draw_texture(tex, x, y, clip);

    internal__draw_separator(internal__get_relative_cursor(ui_panels.top()), ui_panels.top().cursor_dir, w, h, ui_color_med_dark);
    internal__advance_ui_cursor_end(ui_panels.top(), w, h);

    // If we are currently hot then we push our info to the status bar.
    if (!locked && !info.empty() && internal__is_hot())
    {
        std::string kb_info;
        if (kb.empty())
        {
            kb_info = FormatString("(%s)", get_key_binding_main_string(kb).c_str());
            if (get_key_binding(kb).alt_code && get_key_binding(kb).alt_mod)
            {
                kb_info += FormatString(" or (%s)", get_key_binding_alt_string(kb).c_str());
            }
        }
        std::string info_text((kb_info.empty()) ? info : FormatString("%s %s", kb_info.c_str(), info.c_str()));
        push_status_bar_message(info_text.c_str());
    }
    // If we are currently hot then set the tooltip.
    if (!locked && !name.empty() && internal__is_hot())
    {
        set_current_tooltip(name);
    }

    ++ui_current_id;

    return result;
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
