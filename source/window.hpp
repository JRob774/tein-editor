/*******************************************************************************
 * Window manager for creating and handling multiple different app windows.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

/* GLOBAL WINDOW FUNCTIONALITY ************************************************/

static constexpr const char* MAIN_WINDOW_TITLE  = "The End is Nigh: Editor";
static constexpr int         MAIN_WINDOW_X      = SDL_WINDOWPOS_CENTERED;
static constexpr int         MAIN_WINDOW_Y      = SDL_WINDOWPOS_CENTERED;
static constexpr int         MAIN_WINDOW_BASE_W = 1280;
static constexpr int         MAIN_WINDOW_BASE_H =  722; // +2 so the info panel doesn't shift the layer panel!
static constexpr int         MAIN_WINDOW_MIN_W  =  600;
static constexpr int         MAIN_WINDOW_MIN_H  =  320;
static constexpr u32         MAIN_WINDOW_FLAGS  = SDL_WINDOW_RESIZABLE;

static constexpr float WINDOW_BORDER = 4.0f;

typedef u32 Window_ID;
struct      Window;

FILDEF bool init_window ();
FILDEF void quit_window ();

FILDEF void handle_window_events ();

INLDEF void set_main_window_subtitle (std::string subtitle);

FILDEF void show_main_window ();

FILDEF Window&     get_focused_window      (                );
FILDEF Window&     get_window              (std::string name);
FILDEF Window&     get_window_from_id      (Window_ID     id);
FILDEF Window_ID   get_window_id           (std::string name);
FILDEF std::string get_window_name_from_id (Window_ID     id);

FILDEF bool is_a_window_resizing ();

/******************************************************************************/

/* -------------------------------------------------------------------------- */

/* INDIVIDUAL WINDOW FUNCTIONALITY ********************************************/

typedef void(*Window_Resize_Callback)(void);
typedef void(*Window_Close_Callback)(void);

struct Window
{
    Window_Resize_Callback resize_callback;
    Window_Close_Callback close_callback;

    SDL_Window* window;
    Window_ID id;

    bool focus;
    bool mouse;

    float width;
    float height;
};

STDDEF bool create_window (std::string name, std::string title, int x, int y, int w, int h,
                           int min_w = 0, int min_h = 0, u32 flags = 0);

FILDEF bool is_window_hidden  (std::string name);
FILDEF bool is_window_focused (std::string name);

FILDEF void show_window  (std::string name);
FILDEF void raise_window (std::string name);
FILDEF void hide_window  (std::string name);

FILDEF void set_window_title    (std::string name, std::string title);
FILDEF void set_window_min_size (std::string name, int w, int h);
FILDEF void set_window_max_size (std::string name, int w, int h);
FILDEF void set_window_pos      (std::string name, int x, int y);
FILDEF void set_window_size     (std::string name, int w, int h);
FILDEF void set_window_child    (std::string name);

/******************************************************************************/

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
