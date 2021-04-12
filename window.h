#ifndef __VTK_WINDOW_H__
#define __VTK_WINDOW_H__

#include <stdbool.h>
#include <cairo.h>
#include <X11/Xlib.h>
#include "vtk.h"

struct vtk_window {
	vtk root;
	Window w;
	long event_mask;
	bool should_close;
	int updatefd;

	cairo_surface_t *csurf;
	cairo_t *cr;

	struct vtk_event_handlers {
		struct { vtk_event_handler h; void *d; }
			close,
			draw,
			key_press,
			key_release,
			mouse_move,
			mouse_press,
			mouse_release,
			resize,
			scroll,
			update;
	} event;
};

#endif
