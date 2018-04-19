#include <stdlib.h>
#include <X11/Xutil.h>
#include "root.h"
#include "vtk.h"

vtk_err vtk_new(vtk *root) {
	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) {
		return VTK_XOPENDISPLAY_FAILED;
	}

	XVisualInfo visinfo = {
		.screen = DefaultScreen(dpy),
		.class = TrueColor,
	};

	int nvisinfo;
	XVisualInfo *visinfos = XGetVisualInfo(dpy, VisualScreenMask | VisualClassMask, &visinfo, &nvisinfo);
	if (!visinfos) {
		return VTK_NO_SUITABLE_VISUAL;
	}

	visinfo.depth = 0;
	for (int i = 0; i < nvisinfo; i++) {
		if (visinfos[i].depth > visinfo.depth) {
			visinfo = visinfos[i];
		}
	}


	*root = malloc(sizeof **root);
	if (!*root) {
		return VTK_ALLOCATION_FAILED;
	}

	(*root)->dpy = dpy;
	(*root)->visual = visinfo.visual;
	(*root)->depth = visinfo.depth;
	(*root)->cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), visinfo.visual, AllocNone);
	(*root)->wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	return 0;
}

void vtk_destroy(vtk root) {
	XCloseDisplay(root->dpy);
	free(root);
}
