#pragma once

static constexpr int gAppVerMajor = 1;
static constexpr int gAppVerMinor = 1;
static constexpr int gAppVerPatch = 0;

static SDL_Event gMainEvent;
static bool gMainRunning;

TEINAPI void InitApplication (int argc, char** argv);
TEINAPI void QuitApplication ();

TEINAPI void DoApplication ();

TEINAPI bool HandleApplicationEvents ();
