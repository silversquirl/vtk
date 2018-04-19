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

	cairo_surface_t *csurf;
	cairo_t *cr;

	struct vtk_event_handlers {
		vtk_event_handler
			close,
			draw,
			resize;
		void *data;
	} event;
};

#endif
