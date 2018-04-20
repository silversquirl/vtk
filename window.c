#include <stdlib.h>
#include <cairo-xlib.h>
#include "debug.h"
#include "event.h"
#include "root.h"
#include "window.h"
#include "vtk.h"

vtk_err vtk_window_new(vtk_window *win, vtk root, const char *title, int x, int y, int width, int height) {
	int black = BlackPixel(root->dpy, DefaultScreen(root->dpy));
	XSetWindowAttributes wattr = {
		.colormap = root->cmap,
		.background_pixel = black,
		.border_pixel = black,
	};

	Window w = XCreateWindow(
		root->dpy, DefaultRootWindow(root->dpy),
		x, y, width, height, 0,
		root->depth, InputOutput, root->visual,
		CWColormap | CWBackPixel | CWBorderPixel, &wattr);
	XStoreName(root->dpy, w, title);
	XSetWMProtocols(root->dpy, w, &root->wm_delete_window, 1);

	cairo_surface_t *csurf = cairo_xlib_surface_create(root->dpy, w, root->visual, width, height);
	cairo_t *cr = cairo_create(csurf);

	*win = malloc(sizeof **win);
	if (!*win) {
		return VTK_ALLOCATION_FAILED;
	}

	(*win)->root = root;
	(*win)->w = w;
	(*win)->event_mask = StructureNotifyMask; // We want to handle at least resize events
	(*win)->should_close = false;

	(*win)->csurf = csurf;
	(*win)->cr = cr;

	// Set all the event handlers to NULL
	(*win)->event = (struct vtk_event_handlers){ NULL };

	return 0;
}

void vtk_window_destroy(vtk_window win) {
	cairo_destroy(win->cr);
	cairo_surface_destroy(win->csurf);
	XDestroyWindow(win->root->dpy, win->w);
	free(win);
}

void vtk_window_close(vtk_window win) { win->should_close = true; }

void vtk_window_redraw(vtk_window win) {
	cairo_push_group(win->cr);
	win->event.draw((vtk_event){ VTK_EV_DRAW }, win->event.data);
	cairo_pop_group_to_source(win->cr);
	XClearWindow(win->root->dpy, win->w);
	cairo_paint(win->cr);
	cairo_surface_flush(win->csurf);
	XFlush(win->root->dpy);
}

void vtk_window_mainloop(vtk_window win) {
	XSelectInput(win->root->dpy, win->w, win->event_mask);
	XMapWindow(win->root->dpy, win->w);

	XEvent ev;
	while (!win->should_close) {
		XNextEvent(win->root->dpy, &ev);
		vtk_event_handle(win, ev);
	}
}

void vtk_window_set_title(vtk_window win, const char *title) {
	XStoreName(win->root->dpy, win->w, title);
}

void vtk_window_get_size(vtk_window win, int *width, int *height) {
	if (width) {
		*width = cairo_xlib_surface_get_width(win->csurf);
	}
	if (height) {
		*height = cairo_xlib_surface_get_height(win->csurf);
	}
}

cairo_t *vtk_window_get_cairo(vtk_window win) { return win->cr; }

void vtk_window_set_event_handler(vtk_window win, vtk_event_type type, vtk_event_handler cb) {
	switch (type) {
	case VTK_EV_CLOSE:
		win->event.close = cb;
		break;

	case VTK_EV_DRAW:
		win->event.draw = cb;
		win->event_mask |= ExposureMask;
		break;

	case VTK_EV_KEY_PRESS:
		win->event.key_press = cb;
		win->event_mask |= KeyPressMask;
		break;

	case VTK_EV_KEY_RELEASE:
		win->event.key_release = cb;
		win->event_mask |= KeyReleaseMask;
		break;

	case VTK_EV_MOUSE_MOVE:
		win->event.mouse_move = cb;
		win->event_mask |= PointerMotionMask;
		break;

	case VTK_EV_MOUSE_PRESS:
		win->event.mouse_press = cb;
		win->event_mask |= ButtonPressMask;
		break;

	case VTK_EV_MOUSE_RELEASE:
		win->event.mouse_release = cb;
		win->event_mask |= ButtonReleaseMask;
		break;

	case VTK_EV_RESIZE:
		win->event.resize = cb;
		break;

	case VTK_EV_SCROLL:
		win->event.scroll = cb;
		if (win->root->xi2.enable) {
			DEBUG("XISelectEvents");
			XISelectEvents(win->root->dpy, win->w, &win->root->xi2.emask, 1);
		} else {
			win->event_mask |= ButtonPressMask;
		}
		break;
	}
}

void vtk_window_set_event_handler_data(vtk_window win, void *data) {
	win->event.data = data;
}
