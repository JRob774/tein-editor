/*******************************************************************************
 * System that checks for updated versions and displays the changes/features.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// GitHub Releases API: https://developer.github.com/v3/repos/releases/#get-the-slatest-release
static constexpr const char* GITHUB_API_USER_AGENT = "tein-editor";
static constexpr const char* GITHUB_API_URL = "https://api.github.com/repos/JRob774/tein-editor/releases/latest";

static constexpr const char* DOWNLOAD_PAGE = "https://jrob774.itch.io/tein-editor";

static constexpr int HTTP_CODE_OK      = 200;
static constexpr int HTTP_CODE_CREATED = 201;

static constexpr float UPDATE_DIALOG_BOTTOM_BORDER =   26;
static constexpr float UPDATE_DIALOG_WAIT_TIME     = .33f; // Seconds

static CURL*                 curl;
static SDL_TimerID   update_timer;
static bool    there_is_an_update;
static nlohmann::json update_json;

static std::string update_title;
static std::string update_body;

static float update_dialog_content_height;
static float update_dialog_scroll_offset;

/* -------------------------------------------------------------------------- */

FILDEF u32 internal__update_show_callback (u32 interval, void* user_data)
{
    push_editor_event(EDITOR_EVENT_SHOW_UPDATE, NULL, NULL);
    return 0;
}

FILDEF void internal__update_dialog_update ()
{
    load_webpage(DOWNLOAD_PAGE);
    hide_window("WINUPDATE");
}

FILDEF void internal__update_dialog_cancel ()
{
    hide_window("WINUPDATE");
}

FILDEF size_t internal__curl_write_callback (const char* in, size_t size, size_t num, std::string* out)
{
    size_t total_bytes = size * num;
    out->append(in, total_bytes);
    return total_bytes;
}

/* -------------------------------------------------------------------------- */

// Example Curl GET <https://gist.github.com/connormanning/41efa6075515019e499c>
FILDEF void check_for_updates ()
{
    LOG_DEBUG("Checking for new releases...");

    curl = curl_easy_init();
    if (!curl)
    {
        LOG_ERROR(ERR_MIN, "Failed to initialize Curl!");
        return;
    }
    defer { curl_easy_cleanup(curl); };

    curl_easy_setopt(curl, CURLOPT_USERAGENT,      GITHUB_API_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_URL,            GITHUB_API_URL       );
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE,      CURL_IPRESOLVE_V4    );
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        10);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    long http_code = 0;
    std::unique_ptr<std::string> http_data(new std::string());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal__curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     http_data.get());

    // Run our HTTP GET command and capture the HTTP response code.
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != HTTP_CODE_OK && http_code != HTTP_CODE_CREATED)
    {
        LOG_ERROR(ERR_MIN, "Failed to GET update information (%d)!", http_code);
        return;
    }

    update_json = nlohmann::json::parse(*http_data.get());

    std::string current_version(format_string("v%d.%d.%d", APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));
    std::string latest_version(current_version);
    if (update_json.find("tag_name") != update_json.end())
    {
        latest_version = update_json["tag_name"].get<std::string>();
    }

    if (current_version != latest_version)
    {
        LOG_DEBUG("New Release Found");
        there_is_an_update = true;
    }
    else
    {
        LOG_DEBUG("No New Releases");
        there_is_an_update = false;
    }
}

FILDEF void open_update_window_timed ()
{
    // Looked weird showing immediately so we put it on a timer.
    u32 interval = static_cast<u32>((UPDATE_DIALOG_WAIT_TIME * 1000));
    update_timer = SDL_AddTimer(interval, internal__update_show_callback, NULL);
    if (!update_timer) open_update_window(); // Just show it immediately.
}

FILDEF void open_update_window ()
{
    update_title = update_json["name"].get<std::string>();
    update_body  = update_json["body"].get<std::string>();

    // Remove the first line because we don't need that bit.
    update_body.erase(0, update_body.find("\n")+1);

    update_dialog_scroll_offset = 0;

    show_window("WINUPDATE");
}

FILDEF void do_update ()
{
    quad p1, p2;

    p1.x = WINDOW_BORDER;
    p1.y = WINDOW_BORDER;
    p1.w = get_viewport().w - (WINDOW_BORDER * 2);
    p1.h = get_viewport().h - (WINDOW_BORDER * 2);

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float bb = PATH_DIALOG_BOTTOM_BORDER;

    float vw = get_viewport().w;
    float vh = get_viewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - WINDOW_BORDER;

    // Bottom buttons for okaying or cancelling the path dialog.
    vec2 btn_cursor(0, WINDOW_BORDER);
    begin_panel(0, vh-bb, vw, bb, UI_NONE, ui_color_medium);

    set_panel_cursor_dir(UI_DIR_RIGHT);
    set_panel_cursor(&btn_cursor);

    set_ui_font(&get_editor_regular_font());

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (do_button_txt(NULL, bw ,bh, UI_NONE, "Download")) internal__update_dialog_update();
    if (do_button_txt(NULL, bw2,bh, UI_NONE, "Cancel"  )) internal__update_dialog_cancel();

    // Add a separator to the left for symmetry.
    btn_cursor.x = 1;
    do_separator(bh);

    end_panel();

    p2.x =                  1;
    p2.y =                  1;
    p2.w = vw             - 2;
    p2.h = vh - p2.y - bb - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    constexpr float HEADER_SECTION = 68;

    constexpr float XPAD1 = 8;
    constexpr float YPAD1 = 8;

    vec2 cursor(XPAD1, YPAD1);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    constexpr float MAIN_H = 24;
    constexpr float SUB_H = 20;

    set_font_point_size(get_editor_bold_font(), LARGE_FONT_POINT_SIZE);
    set_ui_font(&get_editor_bold_font());
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, MAIN_H, update_title);

    cursor.y -= 2;

    set_font_point_size(get_editor_regular_font(), SMALL_FONT_POINT_SIZE);
    set_ui_font(&get_editor_regular_font());
    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, SUB_H, "There is a new editor version available!");

    cursor.y = YPAD1 + HEADER_SECTION - SUB_H;

    set_font_point_size(get_editor_bold_font(), SMALL_FONT_POINT_SIZE);
    set_ui_font(&get_editor_bold_font());
    do_label(UI_ALIGN_LEFT,UI_ALIGN_BOTTOM, SUB_H, "Version Changelog:");

    constexpr float SCROLLBAR_WIDTH = 12;

    constexpr float XPAD2 = 4;
    constexpr float YPAD2 = 4;

    float px = XPAD2;
    float py = YPAD1 + HEADER_SECTION + roundf(YPAD1*.66f);
    float pw = get_viewport().w - (XPAD2*2) - 1 - SCROLLBAR_WIDTH;
    float ph = get_viewport().h - py - YPAD2;

    begin_panel(px,py,pw,ph, UI_NONE, ui_color_ex_dark);

    constexpr float XPAD3 = 6;
    constexpr float YPAD3 = 4;

    begin_panel(XPAD3,YPAD3,pw-(XPAD3*2),ph-(YPAD3*2), UI_NONE);

    vec2 inner_cursor(0,0);
    set_panel_cursor(&inner_cursor);

    set_font_point_size(get_editor_regular_font(), SMALL_FONT_POINT_SIZE);
    set_font_point_size(get_editor_bold_font(), SMALL_FONT_POINT_SIZE);

    float label_w = get_viewport().w;
    update_dialog_content_height = get_markdown_h(label_w, update_body) + YPAD3;
    float sx = get_viewport().w + XPAD3 + 1;
    float sy = 0 - YPAD3;
    float sw = SCROLLBAR_WIDTH;
    float sh = get_viewport().h + (YPAD3*2);
    do_scrollbar(sx, sy, sw, sh, update_dialog_content_height, update_dialog_scroll_offset);

    do_markdown(label_w, update_dialog_content_height, update_body);

    end_panel();
    end_panel();
    end_panel();
    end_panel();

    // Reset these so they don't interfere with any other part of the editor.
    set_font_point_size(get_editor_regular_font(), SMALL_FONT_POINT_SIZE);
    set_font_point_size(get_editor_bold_font(), SMALL_FONT_POINT_SIZE);
}

FILDEF void handle_update_events ()
{
    if (main_event.type == SDL_USEREVENT)
    {
        if (main_event.user.code == EDITOR_EVENT_SHOW_UPDATE)
        {
            open_update_window();
        }
    }

    if (!is_window_focused("WINUPDATE")) return;

    if (main_event.type == SDL_KEYDOWN)
    {
        if (main_event.key.keysym.sym == SDLK_RETURN)
        {
            internal__update_dialog_update();
        }
        else if (main_event.key.keysym.sym == SDLK_ESCAPE)
        {
            internal__update_dialog_cancel();
        }
    }
}

FILDEF bool are_there_updates ()
{
    return there_is_an_update;
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
