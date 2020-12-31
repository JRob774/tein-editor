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

TEINAPI U32 internal__update_show_callback (U32 interval, void* user_data)
{
    PushEditorEvent(EDITOR_EVENT_SHOW_UPDATE, NULL, NULL);
    return 0;
}

TEINAPI void internal__update_dialog_update ()
{
    LoadWebpage(DOWNLOAD_PAGE);
    HideWindow("WINUPDATE");
}

TEINAPI void internal__update_dialog_cancel ()
{
    HideWindow("WINUPDATE");
}

TEINAPI size_t internal__curl_write_callback (const char* in, size_t size, size_t num, std::string* out)
{
    size_t total_bytes = size * num;
    out->append(in, total_bytes);
    return total_bytes;
}

/* -------------------------------------------------------------------------- */

// Example Curl GET <https://gist.github.com/connormanning/41efa6075515019e499c>
TEINAPI void check_for_updates ()
{
    LogDebug("Checking for new releases...");

    curl = curl_easy_init();
    if (!curl)
    {
        LogError(ERR_MIN, "Failed to initialize Curl!");
        return;
    }
    Defer { curl_easy_cleanup(curl); };

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
        LogError(ERR_MIN, "Failed to GET update information (%d)!", http_code);
        return;
    }

    update_json = nlohmann::json::parse(*http_data.get());

    std::string current_version(FormatString("v%d.%d.%d", gAppVerMajor,gAppVerMinor,gAppVerPatch));
    std::string latest_version(current_version);
    if (update_json.find("tag_name") != update_json.end())
    {
        latest_version = update_json["tag_name"].get<std::string>();
    }

    if (current_version != latest_version)
    {
        LogDebug("New Release Found");
        there_is_an_update = true;
    }
    else
    {
        LogDebug("No New Releases");
        there_is_an_update = false;
    }
}

TEINAPI void open_update_window_timed ()
{
    // Looked weird showing immediately so we put it on a timer.
    U32 interval = static_cast<U32>((UPDATE_DIALOG_WAIT_TIME * 1000));
    update_timer = SDL_AddTimer(interval, internal__update_show_callback, NULL);
    if (!update_timer) open_update_window(); // Just show it immediately.
}

TEINAPI void open_update_window ()
{
    update_title = update_json["name"].get<std::string>();
    update_body  = update_json["body"].get<std::string>();

    // Remove the first line because we don't need that bit.
    update_body.erase(0, update_body.find("\n")+1);

    update_dialog_scroll_offset = 0;

    ShowWindow("WINUPDATE");
}

TEINAPI void do_update ()
{
    Quad p1, p2;

    p1.x = gWindowBorder;
    p1.y = gWindowBorder;
    p1.w = GetViewport().w - (gWindowBorder * 2);
    p1.h = GetViewport().h - (gWindowBorder * 2);

    BeginPanel(p1, UI_NONE, gUiColorExDark);

    float bb = gPathDialogBottomBorder;

    float vw = GetViewport().w;
    float vh = GetViewport().h;

    float bw = roundf(vw / 2);
    float bh = bb - gWindowBorder;

    // Bottom buttons for okaying or cancelling the path dialog.
    Vec2 btn_cursor(0, gWindowBorder);
    BeginPanel(0, vh-bb, vw, bb, UI_NONE, gUiColorMedium);

    SetPanelCursorDir(UI_DIR_RIGHT);
    SetPanelCursor(&btn_cursor);

    SetUiFont(&GetEditorRegularFont());

    // Just to make sure that we always reach the end of the panel space.
    float bw2 = vw - bw;

    if (DoTextButton(NULL, bw ,bh, UI_NONE, "Download")) internal__update_dialog_update();
    if (DoTextButton(NULL, bw2,bh, UI_NONE, "Cancel"  )) internal__update_dialog_cancel();

    // Add a separator to the left for symmetry.
    btn_cursor.x = 1;
    DoSeparator(bh);

    EndPanel();

    p2.x =                  1;
    p2.y =                  1;
    p2.w = vw             - 2;
    p2.h = vh - p2.y - bb - 1;

    BeginPanel(p2, UI_NONE, gUiColorMedium);

    constexpr float HEADER_SECTION = 68;

    constexpr float XPAD1 = 8;
    constexpr float YPAD1 = 8;

    Vec2 cursor(XPAD1, YPAD1);

    SetPanelCursorDir(UI_DIR_DOWN);
    SetPanelCursor(&cursor);

    constexpr float MAIN_H = 24;
    constexpr float SUB_H = 20;

    SetFontPointSize(GetEditorBoldFont(), gLargeFontPointSize);
    SetUiFont(&GetEditorBoldFont());
    DoLabel(UI_ALIGN_LEFT,UI_ALIGN_CENTER, MAIN_H, update_title);

    cursor.y -= 2;

    SetFontPointSize(GetEditorRegularFont(), gSmallFontPointSize);
    SetUiFont(&GetEditorRegularFont());
    DoLabel(UI_ALIGN_LEFT,UI_ALIGN_CENTER, SUB_H, "There is a new editor version available!");

    cursor.y = YPAD1 + HEADER_SECTION - SUB_H;

    SetFontPointSize(GetEditorBoldFont(), gSmallFontPointSize);
    SetUiFont(&GetEditorBoldFont());
    DoLabel(UI_ALIGN_LEFT,UI_ALIGN_BOTTOM, SUB_H, "Version Changelog:");

    constexpr float SCROLLBAR_WIDTH = 12;

    constexpr float XPAD2 = 4;
    constexpr float YPAD2 = 4;

    float px = XPAD2;
    float py = YPAD1 + HEADER_SECTION + roundf(YPAD1*.66f);
    float pw = GetViewport().w - (XPAD2*2) - 1 - SCROLLBAR_WIDTH;
    float ph = GetViewport().h - py - YPAD2;

    BeginPanel(px,py,pw,ph, UI_NONE, gUiColorExDark);

    constexpr float XPAD3 = 6;
    constexpr float YPAD3 = 4;

    BeginPanel(XPAD3,YPAD3,pw-(XPAD3*2),ph-(YPAD3*2), UI_NONE);

    Vec2 inner_cursor(0,0);
    SetPanelCursor(&inner_cursor);

    SetFontPointSize(GetEditorRegularFont(), gSmallFontPointSize);
    SetFontPointSize(GetEditorBoldFont(), gSmallFontPointSize);

    float label_w = GetViewport().w;
    update_dialog_content_height = GetMarkdownHeight(label_w, update_body) + YPAD3;
    float sx = GetViewport().w + XPAD3 + 1;
    float sy = 0 - YPAD3;
    float sw = SCROLLBAR_WIDTH;
    float sh = GetViewport().h + (YPAD3*2);
    DoScrollbar(sx, sy, sw, sh, update_dialog_content_height, update_dialog_scroll_offset);

    DoMarkdown(label_w, update_dialog_content_height, update_body);

    EndPanel();
    EndPanel();
    EndPanel();
    EndPanel();

    // Reset these so they don't interfere with any other part of the editor.
    SetFontPointSize(GetEditorRegularFont(), gSmallFontPointSize);
    SetFontPointSize(GetEditorBoldFont(), gSmallFontPointSize);
}

TEINAPI void handle_update_events ()
{
    if (gMainEvent.type == SDL_USEREVENT)
    {
        if (gMainEvent.user.code == EDITOR_EVENT_SHOW_UPDATE)
        {
            open_update_window();
        }
    }

    if (!IsWindowFocused("WINUPDATE")) return;

    if (gMainEvent.type == SDL_KEYDOWN)
    {
        if (gMainEvent.key.keysym.sym == SDLK_RETURN)
        {
            internal__update_dialog_update();
        }
        else if (gMainEvent.key.keysym.sym == SDLK_ESCAPE)
        {
            internal__update_dialog_cancel();
        }
    }
}

TEINAPI bool are_there_updates ()
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
