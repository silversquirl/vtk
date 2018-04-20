#ifndef __VTK_ROOT_H__
#define __VTK_ROOT_H__

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

struct vtk_root {
	Display *dpy;
	Visual *visual;
	int depth;
	Colormap cmap;
	Atom wm_delete_window;

	struct {
		bool enable;
		int opcode, device;
		XIEventMask emask;
		unsigned char evt_mask_dat[1];

		struct {
			int valuator;
			double increment;

			bool reset;
			double value;
		} scroll_v;
	} xi2;
};

bool vtk_root_update_xi2_scroll(struct vtk_root *root, XIAnyClassInfo **classes, int nclass);

#endif
