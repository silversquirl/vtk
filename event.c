#include <limits.h>
#include <strings.h>
#include <cairo-xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>
#include "debug.h"
#include "event.h"
#include "root.h"
#include "window.h"
#include "vtk.h"

static vtk_modifiers _vtk_modifiers(unsigned int state) {
	vtk_modifiers vm = 0;
	if (state & ShiftMask)	vm |= VTK_M_SHIFT;
	if (state & LockMask)	vm |= VTK_M_CAPS_LOCK;
	if (state & ControlMask)	vm |= VTK_M_CONTROL;
	if (state & Mod1Mask)	vm |= VTK_M_ALT;
	if (state & Mod4Mask)	vm |= VTK_M_SUPER;

	if (state & Button1Mask)	vm |= VTK_M_LEFT_BTN;
	if (state & Button2Mask)	vm |= VTK_M_MIDDLE_BTN;
	if (state & Button3Mask)	vm |= VTK_M_RIGHT_BTN;

	return vm;
}

static void _vtk_event_key(vtk_window win, XKeyEvent ev) {
	int shiftlvl = (ev.state & ShiftMask) == (ev.state & LockMask) ? 0 : 1;
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
			DEBUG("Unknown KeySym received: %lu", xk);
		}
		break;
	}

	vtk_event ve;
	ve.key = (struct vtk_key_event){
		.type = ev.type == KeyPress ? VTK_EV_KEY_PRESS : VTK_EV_KEY_RELEASE,
		.key = vk,
		.mods = _vtk_modifiers(ev.state)
	};

	switch (ev.type) {
	case KeyPress:
		if (win->event.key_press.h) {
			win->event.key_press.h(ve, win->event.key_press.d);
		}
		break;
	case KeyRelease:
		if (win->event.key_release.h) {
			win->event.key_release.h(ve, win->event.key_release.d);
		}
		break;
	}
}

static void _vtk_event_motion(vtk_window win, XMotionEvent ev) {
	if (!win->event.mouse_move.h) return;
	vtk_event ve;
	ve.mouse_move = (struct vtk_mouse_move_event){
		.type = VTK_EV_MOUSE_MOVE,
		.mods = _vtk_modifiers(ev.state),
		.x = ev.x,
		.y = ev.y,
	};
	win->event.mouse_move.h(ve, win->event.mouse_move.d);
}

static void _vtk_event_send_scroll(vtk_window win, double amount) {
	DEBUG("scroll");
	if (!win->event.scroll.h) return;
	vtk_event ve;
	ve.scroll = (struct vtk_scroll_event){
		.type = VTK_EV_SCROLL,
		.amount = amount,
	};
	win->event.scroll.h(ve, win->event.scroll.d);
}

static void _vtk_event_button(vtk_window win, XButtonEvent ev) {
	vtk_modifiers vb;
	switch (ev.button) {
	case Button1:
		vb = VTK_M_LEFT_BTN;
		break;
	case Button2:
		vb = VTK_M_MIDDLE_BTN;
		break;
	case Button3:
		vb = VTK_M_RIGHT_BTN;
		break;
	case Button4:
		if (!win->root->xi2.enable) {
			return _vtk_event_send_scroll(win, 1);
		}
	case Button5:
		if (!win->root->xi2.enable) {
			return _vtk_event_send_scroll(win, -1);
		}
	}

	vtk_event ve;
	ve.mouse_button = (struct vtk_mouse_button_event){
		.type = ev.type == ButtonPress ? VTK_EV_MOUSE_PRESS : VTK_EV_MOUSE_RELEASE,
		.btn = vb,
		.mods = _vtk_modifiers(ev.state),
		.x = ev.x,
		.y = ev.y,
	};

	switch (ev.type) {
	case ButtonPress:
		if (win->event.mouse_press.h) {
			win->event.mouse_press.h(ve, win->event.mouse_press.d);
		}
		break;
	case ButtonRelease:
		if (win->event.mouse_release.h) {
			win->event.mouse_release.h(ve, win->event.mouse_release.d);
		}
		break;
	}
}

static void _vtk_event_configure(vtk_window win, XConfigureEvent ev) {
	int w, h;
	vtk_window_get_size(win, &w, &h);
	if (ev.width != w || ev.height != h) {
		cairo_xlib_surface_set_size(win->csurf, ev.width, ev.height);
		if (win->event.resize.h) {
			win->event.resize.h((vtk_event){ VTK_EV_RESIZE }, win->event.resize.d);
		}
		vtk_window_redraw(win);
	}
}

static void _vtk_event_scroll(vtk_window win, XGenericEventCookie cookie) {
	union {
		XIDeviceChangedEvent *devchange;
		XIDeviceEvent *dev;
		XIEnterEvent *enter;
	} ev = { cookie.data };

	switch (cookie.evtype) {
	case XI_DeviceChanged:
		DEBUG("DeviceChanged");
		vtk_root_update_xi2_scroll(win->root, ev.devchange->classes, ev.devchange->num_classes);
		break;

	case XI_Enter:
		if (ev.enter->evtype == XI_Enter && ev.enter->mode == XINotifyNormal) {
			DEBUG("Enter");
			win->root->xi2.scroll_v.reset = true;
		}
		break;

	case XI_Motion:
		for (int byte_i = 0, val_i = 0; byte_i < ev.dev->valuators.mask_len; byte_i++) {
			int cur_byte = ev.dev->valuators.mask[byte_i], bit_i;
			int valuator = CHAR_BIT * byte_i - 1; // - 1 accounts for ffs starting at 1

			// Loop through each set bit
			while ((bit_i = ffs(cur_byte))) {
				valuator += bit_i;
				cur_byte >>= bit_i;

				if (valuator > win->root->xi2.scroll_v.valuator) goto finish;
				if (valuator == win->root->xi2.scroll_v.valuator) {
					double value = ev.dev->valuators.values[val_i];
					if (win->root->xi2.scroll_v.reset) {
						DEBUG("Resetting stored scroll value");
						win->root->xi2.scroll_v.reset = false;
					} else {
						double delta = win->root->xi2.scroll_v.value - value;
						_vtk_event_send_scroll(win, delta / win->root->xi2.scroll_v.increment);
					}
					win->root->xi2.scroll_v.value = value;
				}

				val_i++;
			}
		}
finish:
		break;
	}
}

void vtk_event_handle(vtk_window win, XEvent ev) {
	switch (ev.type) {
	case ClientMessage:
		if (ev.xclient.data.l[0] == win->root->wm_delete_window && win->event.close.h) {
			win->event.close.h((vtk_event){ VTK_EV_CLOSE }, win->event.close.d);
		}
		break;

	case Expose:
		vtk_window_redraw(win);
		break;

	case KeyPress:
	case KeyRelease:
		_vtk_event_key(win, ev.xkey);
		break;

	case MotionNotify:
		_vtk_event_motion(win, ev.xmotion);
		break;

	case ButtonPress:
	case ButtonRelease:
		_vtk_event_button(win, ev.xbutton);
		break;

	case ConfigureNotify:
		_vtk_event_configure(win, ev.xconfigure);
		break;

	case GenericEvent:
		if (ev.xgeneric.extension == win->root->xi2.opcode && XGetEventData(win->root->dpy, &ev.xcookie)) {
			_vtk_event_scroll(win, ev.xcookie);
			XFreeEventData(win->root->dpy, &ev.xcookie);
		}
	}
}
