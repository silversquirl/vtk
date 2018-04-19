#include <stdlib.h>
#include <cairo-xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "root.h"
#include "window.h"
#include "vtk.h"
#include "debug.h"

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

static void _vtk_window_key(vtk_window win, XKeyEvent ev) {
	bool shift	= ev.state & ShiftMask;
	bool lock	= ev.state & LockMask;
	bool control	= ev.state & ControlMask;
	bool alt	= ev.state & Mod1Mask;
	bool super	= ev.state & Mod4Mask;

	vtk_modifiers vm = 0;
	if (shift)	{ vm |= VTK_M_SHIFT; }
	if (lock)	{ vm |= VTK_M_CAPS_LOCK; }
	if (control)	{ vm |= VTK_M_CONTROL; }
	if (alt)	{ vm |= VTK_M_ALT; }
	if (super)	{ vm |= VTK_M_SUPER; }

	int shiftlvl = shift != lock ? 1 : 0;
	KeySym xk = XkbKeycodeToKeysym(win->root->dpy, ev.keycode, 0, shiftlvl);

	vtk_key vk;
	switch (xk) {
	case XK_BackSpace:
		vk = VTK_K_BACKSPACE;
		break;

	case XK_Tab:
		vk = VTK_K_TAB;
		break;

	case XK_Return:
		vk = VTK_K_RETURN;
		break;

	case XK_Escape:
		vk = VTK_K_ESCAPE;
		break;

	case XK_Delete:
		vk = VTK_K_DELETE;
		break;

	case XK_Insert:
		vk = VTK_K_INSERT;
		break;

	case XK_Page_Up:
		vk = VTK_K_PAGE_UP;
		break;

	case XK_Page_Down:
		vk = VTK_K_PAGE_DOWN;
		break;

	case XK_Home:
		vk = VTK_K_HOME;
		break;

	case XK_End:
		vk = VTK_K_END;
		break;

	case XK_Up:
		vk = VTK_K_UP;
		break;

	case XK_Down:
		vk = VTK_K_DOWN;
		break;

	case XK_Left:
		vk = VTK_K_LEFT;
		break;

	case XK_Right:
		vk = VTK_K_RIGHT;
		break;

	default:
		if (' ' <= xk && xk <= '~') {
			vk = xk;
		} else {
			DEBUG("Unknown KeySym received: %d", xk);
		}
		break;
	}

	vtk_event ve = {
		.type = ev.type == KeyPress ? VTK_EV_KEY_PRESS : VTK_EV_KEY_RELEASE,
		.key = { vk, vm },
	};

	switch (ev.type) {
	case KeyPress:
		win->event.key_press(ve, win->event.data);
		break;
	case KeyRelease:
		win->event.key_release(ve, win->event.data);
		break;
	}
}

static void _vtk_window_configure_notify(vtk_window win, XConfigureEvent ev) {
	int w, h;
	vtk_window_get_size(win, &w, &h);
	if (ev.width != w || ev.height != h) {
		cairo_xlib_surface_set_size(win->csurf, ev.width, ev.height);
		if (win->event.resize) {
			win->event.resize((vtk_event){ VTK_EV_RESIZE }, win->event.data);
		}
		vtk_window_redraw(win);
	}
}

void vtk_window_mainloop(vtk_window win) {
	XSelectInput(win->root->dpy, win->w, win->event_mask);
	XMapWindow(win->root->dpy, win->w);

	XEvent ev;
	while (!win->should_close) {
		XNextEvent(win->root->dpy, &ev);
		switch (ev.type) {
		case ClientMessage:
			if (ev.xclient.data.l[0] == win->root->wm_delete_window && win->event.close) {
				win->event.close((vtk_event){ VTK_EV_CLOSE }, win->event.data);
			}
			break;

		case Expose:
			vtk_window_redraw(win);
			break;

		case KeyPress:
		case KeyRelease:
			_vtk_window_key(win, ev.xkey);
			break;

		case ConfigureNotify:
			_vtk_window_configure_notify(win, ev.xconfigure);
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

	case VTK_EV_RESIZE:
		win->event.resize = cb;
		break;
	}
}

void vtk_window_set_event_handler_data(vtk_window win, void *data) {
	win->event.data = data;
}
