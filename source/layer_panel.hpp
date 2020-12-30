#pragma once

TEINAPI void InitLayerPanel ();
TEINAPI void DoLayerPanel (bool scrollbar);

TEINAPI float GetLayerPanelHeight ();

TEINAPI bool LayerPanelNeedsScrollbar ();
TEINAPI bool IsLayerPanelPresent ();

TEINAPI void ToggleLayerActive  ();
TEINAPI void ToggleLayerTag     ();
TEINAPI void ToggleLayerOverlay ();
TEINAPI void ToggleLayerBack1   ();
TEINAPI void ToggleLayerBack2   ();
