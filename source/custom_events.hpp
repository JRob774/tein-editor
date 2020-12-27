/*******************************************************************************
 * Custom user-defined events for SDL that are posted by the editor program.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

typedef U32 Editor_Event;

static constexpr U32 EDITOR_EVENT_UI_REDRAW     = 0;
static constexpr U32 EDITOR_EVENT_BACKUP_TAB    = 1;
static constexpr U32 EDITOR_EVENT_HOTLOAD       = 2;
static constexpr U32 EDITOR_EVENT_COOLDOWN      = 3;
static constexpr U32 EDITOR_EVENT_BLINK_CURSOR  = 4;
static constexpr U32 EDITOR_EVENT_GPAK_PROGRESS = 5;
static constexpr U32 EDITOR_EVENT_SHOW_TOOLTIP  = 6;
static constexpr U32 EDITOR_EVENT_SHOW_UPDATE   = 7;

TEINAPI void push_editor_event (Editor_Event id, void* data1, void* data2);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */
