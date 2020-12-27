/*******************************************************************************
 * Interface for loading custom application mouse cursors for the editor.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

static constexpr const char* CURSOR_FILE_ARROW   = "textures/editor_ui/cursor_arrow.bmp";
static constexpr const char* CURSOR_FILE_BEAM_D  = "textures/editor_ui/cursor_beam_dark.bmp";
static constexpr const char* CURSOR_FILE_BEAM_L  = "textures/editor_ui/cursor_beam_light.bmp";
static constexpr const char* CURSOR_FILE_POINTER = "textures/editor_ui/cursor_pointer.bmp";
static constexpr const char* CURSOR_FILE_BRUSH   = "textures/editor_ui/cursor_brush.bmp";
static constexpr const char* CURSOR_FILE_FILL    = "textures/editor_ui/cursor_fill.bmp";
static constexpr const char* CURSOR_FILE_SELECT  = "textures/editor_ui/cursor_select.bmp";

static SDL_Cursor* cursors[Cursor::Total];
static Cursor      current_cursor;
static bool        cursors_enabled;
static bool        cursors_loaded;

/* -------------------------------------------------------------------------- */

TEINAPI bool internal__load_cursor (Cursor cursor, const char* file_name, int x, int y)
{
    SDL_Surface* surface = load_surface_resource(file_name);
    if (!surface)
    {
        LOG_ERROR(ERR_MIN, "Failed to load cursor file '%s'!", file_name);
        return false;
    }
    defer { SDL_FreeSurface(surface); };

    // We need to color key the loaded surface because it is a bitmap so it
    // does not have any alpha transparency by default (our key is white).
    //
    // Whilst we could technically continue even if this part fails we would
    // rather not because no one will want to use cursors with giant white
    // boxes around them... So instead we just fail and instead fallback to
    // using the standard operating system cursor instead of custom cursors.
    u32 color_key = SDL_MapRGB(surface->format, 0xFF,0x00,0xFF);
    if (SDL_SetColorKey(surface, SDL_TRUE, color_key) < 0)
    {
        LOG_ERROR(ERR_MIN, "Failed to color key cursor '%s'!", file_name);
        return false;
    }

    cursors[static_cast<int>(cursor)] = SDL_CreateColorCursor(surface, x, y);
    if (!cursors[static_cast<int>(cursor)])
    {
        LOG_ERROR(ERR_MIN, "Failed to create cursor '%s'!", file_name);
        return false;
    }

    return true;
}

/* -------------------------------------------------------------------------- */

TEINAPI bool load_editor_cursors ()
{
    // We have a special case for the beam cursor as we want a specific
    // version based on whether a light or dark UI is currently loaded.
    const char* CURSOR_FILE_BEAM = (is_ui_light()) ? CURSOR_FILE_BEAM_D : CURSOR_FILE_BEAM_L;

    bool cursors_were_loaded = cursors_loaded;

    // This needs to be here for when we potentially reload the cursors.
    free_editor_cursors();

    cursors_enabled = false;
    cursors_loaded = false;

    if (!internal__load_cursor(Cursor::ARROW,   CURSOR_FILE_ARROW  ,  6,  3)) return false;
    if (!internal__load_cursor(Cursor::BEAM,    CURSOR_FILE_BEAM   , 11, 12)) return false;
    if (!internal__load_cursor(Cursor::POINTER, CURSOR_FILE_POINTER,  8,  1)) return false;
    if (!internal__load_cursor(Cursor::BRUSH,   CURSOR_FILE_BRUSH  ,  3, 20)) return false;
    if (!internal__load_cursor(Cursor::FILL,    CURSOR_FILE_FILL   , 19, 16)) return false;
    if (!internal__load_cursor(Cursor::SELECT,  CURSOR_FILE_SELECT , 11, 12)) return false;

    cursors_enabled = editor_settings.custom_cursors;
    cursors_loaded = true;

    // By default we want to be using the arrow cursor.
    if (!cursors_enabled)
    {
        SDL_SetCursor(SDL_GetDefaultCursor());
        current_cursor = Cursor::ARROW;
    }
    else
    {
        if (cursors_loaded)
        {
            SDL_SetCursor(cursors[static_cast<int>(Cursor::ARROW)]);
            current_cursor = Cursor::ARROW;
        }
    }

    if (!cursors_were_loaded) // Stops this getting printed on settings change.
    {
        LOG_DEBUG("Loaded Editor Cursors");
    }

    return true;
}

TEINAPI void free_editor_cursors ()
{
    for (int i=0; i<static_cast<int>(Cursor::Total); ++i)
    {
        SDL_FreeCursor(cursors[i]);
        cursors[i] = NULL;
    }

    cursors_enabled = false;
    cursors_loaded = false;
}

/* -------------------------------------------------------------------------- */

TEINAPI void set_cursor (Cursor cursor)
{
    if (cursors_enabled && cursors_loaded && current_cursor != cursor && cursors[static_cast<int>(cursor)])
    {
        SDL_SetCursor(cursors[static_cast<int>(cursor)]);
        current_cursor = cursor;
    }
}

TEINAPI Cursor get_cursor ()
{
    return ((cursors_enabled && cursors_loaded) ? current_cursor : Cursor::ARROW);
}

/* -------------------------------------------------------------------------- */

TEINAPI bool custom_cursors_enabled ()
{
    return cursors_enabled;
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
