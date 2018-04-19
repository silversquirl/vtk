#ifndef __VTK_ROOT_H__
#define __VTK_ROOT_H__

#include <X11/Xlib.h>

struct vtk_root {
	Display *dpy;
	Visual *visual;
	int depth;
	Colormap cmap;
	Atom wm_delete_window;
};

#endif
