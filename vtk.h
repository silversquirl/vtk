#ifndef __VTK_H__
#define __VTK_H__

#include <cairo.h>

typedef enum vtk_err {
	VTK_SUCCESS = 0,
	VTK_ALLOCATION_FAILED,
	VTK_NO_SUITABLE_VISUAL,
	VTK_XOPENDISPLAY_FAILED,
} vtk_err;
char *vtk_strerr(vtk_err err);

typedef struct vtk_root *vtk;
vtk_err vtk_new(vtk *root);
void vtk_destroy(vtk root);

typedef struct vtk_window *vtk_window;
vtk_err vtk_window_new(vtk_window *win, vtk root, const char *title, int x, int y, int width, int height);
void vtk_window_destroy(vtk_window win);
void vtk_window_close(vtk_window win);
void vtk_window_redraw(vtk_window win);
void vtk_window_mainloop(vtk_window win);
void vtk_window_set_title(vtk_window win, const char *title);
void vtk_window_get_size(vtk_window win, int *width, int *height);
cairo_t *vtk_window_get_cairo(vtk_window win);

typedef enum vtk_event_type {
	VTK_EV_CLOSE,
	VTK_EV_DRAW,
	VTK_EV_KEY_PRESS,
	VTK_EV_KEY_RELEASE,
	VTK_EV_MOUSE_MOVE,
	VTK_EV_MOUSE_PRESS,
	VTK_EV_MOUSE_RELEASE,
	VTK_EV_RESIZE,
	VTK_EV_SCROLL,
} vtk_event_type;

typedef enum vtk_key {
	VTK_K_NONE,

	VTK_K_BACKSPACE = 0x08,
	VTK_K_TAB = 0x09,
	VTK_K_RETURN = 0x0a,
	VTK_K_ESCAPE = 0x1b,
	VTK_K_SPACE = 0x20,
	VTK_K_DELETE = 0x7f,
	VTK_K_INSERT,

	VTK_K_PAGE_UP,
	VTK_K_PAGE_DOWN,
	VTK_K_HOME,
	VTK_K_END,
	VTK_K_UP,
	VTK_K_DOWN,
	VTK_K_LEFT,
	VTK_K_RIGHT,
} vtk_key;

typedef enum vtk_modifiers {
	VTK_M_SHIFT	= 1 << 0,
	VTK_M_CAPS_LOCK	= 1 << 1,
	VTK_M_CONTROL	= 1 << 2,
	VTK_M_ALT	= 1 << 3,
	VTK_M_SUPER	= 1 << 4,

	VTK_M_LEFT_BTN	= 1 << 5,
	VTK_M_MIDDLE_BTN	= 1 << 6,
	VTK_M_RIGHT_BTN	= 1 << 7,
} vtk_modifiers;

typedef union vtk_event {
	vtk_event_type type;

	struct vtk_key_event {
		vtk_event_type type;
		vtk_key key;	// ASCII character or value from vtk_key
		vtk_modifiers mods;	// ORed mask of values from vtk_modifiers
	} key;

	struct vtk_mouse_move_event {
		vtk_event_type type;
		vtk_modifiers mods;	// ORed mask of values from vtk_modifiers
		int x, y;
	} mouse_move;

	struct vtk_mouse_button_event {
		vtk_event_type type;
		vtk_modifiers btn; // The button that was pressed or released
		vtk_modifiers mods;	// ORed mask of values from vtk_modifiers
		int x, y;
	} mouse_button;

	struct vtk_scroll_event {
		vtk_event_type type;
		double amount;
	} scroll;
} vtk_event;

typedef void (*vtk_event_handler)(vtk_event ev, void *u);

void vtk_window_set_event_handler(vtk_window win, vtk_event_type type, vtk_event_handler handler, void *data);

#endif
