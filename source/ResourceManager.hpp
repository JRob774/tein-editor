// Various clips for the different icons that are found in resources_icons.

static constexpr Quad gClipNone       = {   0,   0,  0,  0 };
static constexpr Quad gClipBullet     = {  96, 108, 24, 12 };
static constexpr Quad gClipBrush      = {   0,   0, 24, 24 };
static constexpr Quad gClipFill       = {  24,   0, 24, 24 };
static constexpr Quad gClipSelect     = {  48,   0, 24, 24 };
static constexpr Quad gClipGrid       = {  72,   0, 24, 24 };
static constexpr Quad gClipBounds     = {   0,  24, 24, 24 };
static constexpr Quad gClipLayers     = {  24,  24, 24, 24 };
static constexpr Quad gClipCamera     = {  48,  24, 24, 24 };
static constexpr Quad gClipFlipH      = {   0,  48, 24, 24 };
static constexpr Quad gClipFlipV      = {  24,  48, 24, 24 };
static constexpr Quad gClipMirrorH    = {  48,  48, 24, 24 };
static constexpr Quad gClipMirrorV    = {  72,  48, 24, 24 };
static constexpr Quad gClipCut        = {   0,  72, 24, 24 };
static constexpr Quad gClipCopy       = {  24,  72, 24, 24 };
static constexpr Quad gClipDeselect   = {  48,  72, 24, 24 };
static constexpr Quad gClipClear      = {  72,  72, 24, 24 };
static constexpr Quad gClipArrowUp    = {  48,  96, 12, 12 };
static constexpr Quad gClipArrowRight = {  60,  96, 12, 12 };
static constexpr Quad gClipArrowLeft  = {  48, 108, 12, 12 };
static constexpr Quad gClipArrowDown  = {  60, 108, 12, 12 };
static constexpr Quad gClipCross      = {  72,  96, 24, 24 };
static constexpr Quad gClipEye        = {  96,  96, 24, 12 };
static constexpr Quad gClipResizeN    = {  24,  96, 12, 12 };
static constexpr Quad gClipResizeE    = {  36,  96, 12, 12 };
static constexpr Quad gClipResizeS    = {  36, 108, 12, 12 };
static constexpr Quad gClipResizeW    = {  24, 108, 12, 12 };
static constexpr Quad gClipResizeNE   = {  12,  96, 12, 12 };
static constexpr Quad gClipResizeSE   = {  12, 108, 12, 12 };
static constexpr Quad gClipResizeSW   = {   0, 108, 12, 12 };
static constexpr Quad gClipResizeNW   = {   0,  96, 12, 12 };
static constexpr Quad gClipResize     = {  72,  24, 24, 24 };
static constexpr Quad gClipPack       = {  96,  24, 24, 24 };
static constexpr Quad gClipUnpack     = {  96,  48, 24, 24 };
static constexpr Quad gClipGuides     = {  96,   0, 24, 24 };
static constexpr Quad gClipEntity     = {  96,  72, 24, 24 };
static constexpr Quad gClipNew        = { 120,   0, 24, 24 };
static constexpr Quad gClipLoad       = { 144,   0, 24, 24 };
static constexpr Quad gClipSave       = { 168,   0, 24, 24 };
static constexpr Quad gClipSaveAs     = { 192,   0, 24, 24 };
static constexpr Quad gClipUndo       = { 216,   0, 24, 24 };
static constexpr Quad gClipRedo       = { 120,  24, 24, 24 };
static constexpr Quad gClipZoomOut    = { 144,  24, 24, 24 };
static constexpr Quad gClipZoomIn     = { 168,  24, 24, 24 };
static constexpr Quad gClipRun        = { 192,  24, 24, 24 };
static constexpr Quad gClipSettings   = { 216,  24, 24, 24 };
static constexpr Quad gClipAbout      = { 120,  48, 24, 24 };
static constexpr Quad gClipHelp       = { 144,  48, 24, 24 };
static constexpr Quad gClipUpdate     = { 168,  48, 24, 24 };
static constexpr Quad gClipBug        = { 192,  48, 24, 24 };

// Globally accessible and generic resources that can be used by the editor.
// We also have a global copy of the no tile/spawn image texture so that we
// can use it in the level editor in case a level is loaded that has IDs we
// no longer recognise -- just so that the user doesn't think they're empty.

static Texture gResourceIcons;

static Font gResourceFontRegularSans;
static Font gResourceFontRegularDyslexic;
static Font gResourceFontRegularLibMono;
static Font gResourceFontMonoDyslexic;
static Font gResourceFontBoldSans;
static Font gResourceFontBoldDyslexic;

static Texture gResourceChecker14;
static Texture gResourceChecker16;
static Texture gResourceChecker20;

static TextureAtlas gResourceLarge;
static TextureAtlas gResourceSmall;

TEINAPI bool InitResourceManager ();
TEINAPI void GetResourceLocation ();

TEINAPI bool            LoadTextureResource (std::string fileName, Texture& texture, TextureWrap wrap = TextureWrap::ClampToEdge);
TEINAPI bool            LoadAtlasResource   (std::string fileName, TextureAtlas& atlas);
TEINAPI bool            LoadFontResource    (std::string fileName, Font& font, std::vector<int> pointSizes = { gSmallFontPointSize }, float cacheSize = gDefaultFontGlyphCacheSize);
TEINAPI Shader          LoadShaderResource  (std::string fileName);
TEINAPI std::vector<U8> LoadBinaryResource  (std::string fileName);
TEINAPI SDL_Surface*    LoadSurfaceResource (std::string fileName);
TEINAPI std::string     LoadStringResource  (std::string fileName);

TEINAPI bool LoadEditorResources ();
TEINAPI void FreeEditorResources ();

TEINAPI std::string BuildResourceString (std::string pathName);

TEINAPI void UpdateEditorFont     ();
TEINAPI bool IsEditorFontOpenSans ();

TEINAPI Font& GetEditorRegularFont ();
TEINAPI Font& GetEditorBoldFont    ();

TEINAPI TextureAtlas& GetEditorAtlasLarge ();
TEINAPI TextureAtlas& GetEditorAtlasSmall ();
