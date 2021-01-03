static constexpr const char* gCursorFileArrow   = "textures/editor_ui/cursor_arrow.bmp";
static constexpr const char* gCursorFileBeamD   = "textures/editor_ui/cursor_beam_dark.bmp";
static constexpr const char* gCursorFileBeamL   = "textures/editor_ui/cursor_beam_light.bmp";
static constexpr const char* gCursorFilePointer = "textures/editor_ui/cursor_pointer.bmp";
static constexpr const char* gCursorFileBrush   = "textures/editor_ui/cursor_brush.bmp";
static constexpr const char* gCursorFileFill    = "textures/editor_ui/cursor_fill.bmp";
static constexpr const char* gCursorFileSelect  = "textures/editor_ui/cursor_select.bmp";

static SDL_Cursor* gCursors[Cursor::Total];
static Cursor gCurrentCursor;

static bool gCursorsEnabled;
static bool gCursorsLoaded;

namespace Internal
{
    TEINAPI bool LoadCursor (Cursor cursor, const char* fileName, int x, int y)
    {
        SDL_Surface* surface = LoadSurfaceResource(fileName);
        if (!surface) {
            LogError(ErrorLevel::Min, "Failed to load cursor file '%s'!", fileName);
            return false;
        }
        Defer { SDL_FreeSurface(surface); };

        // We need to color key the loaded surface because it is a bitmap so it
        // does not have any alpha transparency by default (our key is white).
        //
        // Whilst we could technically continue even if this part fails we would
        // rather not because no one will want to use cursors with giant white
        // boxes around them... So instead we just fail and instead fallback to
        // using the standard operating system cursor instead of custom cursors.
        U32 colorKey = SDL_MapRGB(surface->format, 0xFF,0x00,0xFF);
        if (SDL_SetColorKey(surface, SDL_TRUE, colorKey) < 0) {
            LogError(ErrorLevel::Min, "Failed to color key cursor '%s'!", fileName);
            return false;
        }

        gCursors[static_cast<int>(cursor)] = SDL_CreateColorCursor(surface, x, y);
        if (!gCursors[static_cast<int>(cursor)]) {
            LogError(ErrorLevel::Min, "Failed to create cursor '%s'!", fileName);
            return false;
        }

        return true;
    }
}

TEINAPI bool LoadEditorCursors ()
{
    // We have a special case for the beam cursor as we want a specific
    // version based on whether a light or dark UI is currently loaded.
    const char* CursorFileBeam = (IsUiLight()) ? gCursorFileBeamD : gCursorFileBeamL;

    bool cursorsWereLoaded = gCursorsLoaded;

    // This needs to be here for when we potentially reload the cursors.
    FreeEditorCursors();

    gCursorsEnabled = false;
    gCursorsLoaded = false;

    if (!Internal::LoadCursor(Cursor::Arrow,   gCursorFileArrow  ,  6,  3)) return false;
    if (!Internal::LoadCursor(Cursor::Beam,     CursorFileBeam   , 11, 12)) return false;
    if (!Internal::LoadCursor(Cursor::Pointer, gCursorFilePointer,  8,  1)) return false;
    if (!Internal::LoadCursor(Cursor::Brush,   gCursorFileBrush  ,  3, 20)) return false;
    if (!Internal::LoadCursor(Cursor::Fill,    gCursorFileFill   , 19, 16)) return false;
    if (!Internal::LoadCursor(Cursor::Select,  gCursorFileSelect , 11, 12)) return false;

    gCursorsEnabled = gEditorSettings.customCursors;
    gCursorsLoaded = true;

    // By default we want to be using the arrow cursor.
    if (!gCursorsEnabled) {
        SDL_SetCursor(SDL_GetDefaultCursor());
        gCurrentCursor = Cursor::Arrow;
    } else {
        if (gCursorsLoaded) {
            SDL_SetCursor(gCursors[static_cast<int>(Cursor::Arrow)]);
            gCurrentCursor = Cursor::Arrow;
        }
    }

    if (!cursorsWereLoaded) { // Stops this getting printed on settings change.
        LogDebug("Loaded Editor Cursors");
    }

    return true;
}

TEINAPI void FreeEditorCursors ()
{
    for (int i=0; i<static_cast<int>(Cursor::Total); ++i) {
        SDL_FreeCursor(gCursors[i]);
        gCursors[i] = NULL;
    }
    gCursorsEnabled = false;
    gCursorsLoaded = false;
}

TEINAPI void SetCursorType (Cursor cursor)
{
    if (gCursorsEnabled && gCursorsLoaded && gCurrentCursor != cursor && gCursors[static_cast<int>(cursor)]) {
        SDL_SetCursor(gCursors[static_cast<int>(cursor)]);
        gCurrentCursor = cursor;
    }
}
TEINAPI Cursor GetCursorType ()
{
    return ((gCursorsEnabled && gCursorsLoaded) ? gCurrentCursor : Cursor::Arrow);
}

TEINAPI bool CustomCursorsEnabled ()
{
    return gCursorsEnabled;
}
