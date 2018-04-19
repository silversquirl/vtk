#include <stdlib.h>
#include <cairo-xlib.h>
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
	(*win)->event_handlers = (struct vtk_event_handlers){ NULL };

	return 0;
}

void vtk_window_destroy(vtk_window win) {
	cairo_destroy(win->cr);
	cairo_surface_destroy(win->csurf);
	XDestroyWindow(win->root->dpy, win->w);
	free(win);
}

void vtk_window_close(vtk_window win) { win->should_close = true; }

static void _vtk_window_draw(vtk_window win) {
	cairo_push_group(win->cr);
	win->event_handlers.draw((vtk_event){ VTK_EV_DRAW }, win->event_handlers.draw_data);
	cairo_pop_group_to_source(win->cr);
	XClearWindow(win->root->dpy, win->w);
	cairo_paint(win->cr);
	cairo_surface_flush(win->csurf);
	XFlush(win->root->dpy);
}

static void _vtk_window_configure_notify(vtk_window win, XConfigureEvent ev) {
	int w, h;
	vtk_window_get_size(win, &w, &h);
	if (ev.width != w || ev.height != h) {
		cairo_xlib_surface_set_size(win->csurf, ev.width, ev.height);
		if (win->event_handlers.resize) {
			win->event_handlers.resize((vtk_event){ VTK_EV_RESIZE }, win->event_handlers.resize_data);
		}
		_vtk_window_draw(win);
	}
}

void vtk_window_mainloop(vtk_window win) {
	XSelectInput(win->root->dpy, win->w, win->event_mask);
	XMapWindow(win->root->dpy, win->w);

	XEvent ev;
	while (!win->should_close) {
		XNextEvent(win->root->dpy, &ev);
		switch (ev.type) {
		case Expose:
			_vtk_window_draw(win);
			break;

		case ConfigureNotify:
			_vtk_window_configure_notify(win, ev.xconfigure);
			break;

		case ClientMessage:
			if (ev.xclient.data.l[0] == win->root->wm_delete_window && win->event_handlers.close) {
				win->event_handlers.close((vtk_event){ VTK_EV_CLOSE }, win->event_handlers.close_data);
			}
			break;
		}
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

void vtk_window_set_event_handler(vtk_window win, vtk_event_type type, vtk_event_handler cb, void *ud) {
	switch (type) {
	case VTK_EV_CLOSE:
		win->event_handlers.close = cb;
		win->event_handlers.close_data = ud;
		break;

	case VTK_EV_DRAW:
		win->event_handlers.draw = cb;
		win->event_handlers.draw_data = ud;
		win->event_mask |= ExposureMask;
		break;

	case VTK_EV_RESIZE:
		win->event_handlers.resize = cb;
		win->event_handlers.resize_data = ud;
		break;
	}
}