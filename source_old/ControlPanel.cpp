static Quad gControlPanelBounds;

TEINAPI void DoControlPanel ()
{
    gControlPanelBounds.w = gControlPanelWidth + gControlPanelScrollbarWidth;
    gControlPanelBounds.h = GetViewport().h;
    gControlPanelBounds.x = GetViewport().w - gControlPanelBounds.w;
    gControlPanelBounds.y = 0;

    // Determine if we need scrollbars for either sub-panels.
    bool scrollbar = (TilePanelNeedsScrollbar() || LayerPanelNeedsScrollbar());
    if (!scrollbar) {
        gControlPanelBounds.w = gControlPanelWidth;
        gControlPanelBounds.x = GetViewport().w - gControlPanelBounds.w;
    }

    if (CurrentTabIsLevel()) {
        BeginPanel(gControlPanelBounds, UiFlag::None);
        DoTilePanel(scrollbar);
        DoLayerPanel(scrollbar);
        EndPanel();
    }
}

TEINAPI float GetControlPanelWidth ()
{
    return ((CurrentTabIsLevel()) ? gControlPanelBounds.w : 0);
}
