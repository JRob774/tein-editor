static constexpr float gTileImageSize = 64;

enum TileCategory
{
    TILE_CATEGORY_BASIC,
    TILE_CATEGORY_TAG,
    TILE_CATEGORY_OVERLAY,
    TILE_CATEGORY_ENTITY,
    TILE_CATEGORY_BACK1,
    TILE_CATEGORY_BACK2,
    TILE_CATEGORY_TOTAL
};

TEINAPI bool InitTilePanel ();
TEINAPI void DoTilePanel (bool scrollbar);

TEINAPI bool TilePanelNeedsScrollbar ();

TEINAPI void HandleTilePanelEvents ();

TEINAPI float GetTilePanelHeight ();

TEINAPI void ReloadTileGraphics ();

TEINAPI TileCategory GetSelectedCategory ();
TEINAPI TileID GetSelectedTile ();
TEINAPI LevelLayer GetSelectedLayer ();

TEINAPI LevelLayer CategoryToLayer (TileCategory category);

TEINAPI void SelectNextActiveGroup ();
TEINAPI void SelectPrevActiveGroup ();

TEINAPI void ResetSelectedGroup ();

TEINAPI void IncrementSelectedTile ();
TEINAPI void DecrementSelectedTile ();

TEINAPI void IncrementSelectedGroup ();
TEINAPI void DecrementSelectedGroup ();

TEINAPI void IncrementSelectedCategory ();
TEINAPI void DecrementSelectedCategory ();

TEINAPI TileID GetTileHorizontalFlip (TileID id);
TEINAPI TileID GetTileVerticalFlip   (TileID id);

TEINAPI void JumpToCategoryBasic   ();
TEINAPI void JumpToCategoryTag     ();
TEINAPI void JumpToCategoryOverlay ();
TEINAPI void JumpToCategoryEntity  ();
TEINAPI void JumpToCategoryBack1   ();
TEINAPI void JumpToCategoryBack2   ();
