/*******************************************************************************
 * Custom user-defined events for SDL that are posted by the editor program.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef CUSTOM_EVENTS_HPP__ /*////////////////////////////////////////////////*/
#define CUSTOM_EVENTS_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

typedef u32 Editor_Event;

GLOBAL constexpr u32 EDITOR_EVENT_UI_REDRAW     = 0;
GLOBAL constexpr u32 EDITOR_EVENT_BACKUP_TAB    = 1;
GLOBAL constexpr u32 EDITOR_EVENT_HOTLOAD       = 2;
GLOBAL constexpr u32 EDITOR_EVENT_COOLDOWN      = 3;
GLOBAL constexpr u32 EDITOR_EVENT_BLINK_CURSOR  = 4;
GLOBAL constexpr u32 EDITOR_EVENT_GPAK_PROGRESS = 5;
GLOBAL constexpr u32 EDITOR_EVENT_SHOW_TOOLTIP  = 6;
GLOBAL constexpr u32 EDITOR_EVENT_SHOW_UPDATE   = 7;

FILDEF void push_editor_event (Editor_Event id,
                               void* data1,
                               void* data2);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#endif /* CUSTOM_EVENTS_HPP__ ////////////////////////////////////////////////*/

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