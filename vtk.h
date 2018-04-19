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
void vtk_window_mainloop(vtk_window win);
void vtk_window_set_title(vtk_window win, const char *title);
void vtk_window_get_size(vtk_window win, int *width, int *height);
cairo_t *vtk_window_get_cairo(vtk_window win);

typedef enum vtk_event_type {
	VTK_EV_CLOSE,
	VTK_EV_DRAW,
	VTK_EV_RESIZE,
} vtk_event_type;

typedef struct vtk_event {
	vtk_event_type type;
} vtk_event;

typedef void (*vtk_event_handler)(vtk_event ev, void *u);

void vtk_window_set_event_handler(vtk_window win, vtk_event_type type, vtk_event_handler handler, void *userdata);

#endif
