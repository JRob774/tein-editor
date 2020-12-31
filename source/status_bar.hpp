#pragma once

static constexpr float gStatusBarInnerPad = 6;
static constexpr float gStatusBarHeight = 20;

TEINAPI void PushStatusBarMessage (const char* format, ...);
TEINAPI void DoStatusBar ();
