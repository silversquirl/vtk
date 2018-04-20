#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xutil.h>
#include "root.h"
#include "vtk.h"

bool vtk_root_update_xi2_scroll(vtk root, XIAnyClassInfo **classes, int nclass) {
	for (int i = 0; i < nclass; i++) {
		XIScrollClassInfo *scroll = (XIScrollClassInfo *)classes[i];
		if (scroll->type != XIScrollClass) continue;
		// TODO: horizontal scrolling
		if (scroll->scroll_type != XIScrollTypeVertical) continue;

		root->xi2.scroll_v.valuator = scroll->number;
		root->xi2.scroll_v.increment = scroll->increment;
		root->xi2.scroll_v.reset = true;

		return true;
	}

	return false;
}

static int _vtk_root_xi2_init(vtk root) {
	root->xi2.enable = false;
	int event, error, xi_maj = 2, xi_min = 1;
	if (!XQueryExtension(root->dpy, "XInputExtension", &root->xi2.opcode, &event, &error)) return 1;
	if (XIQueryVersion(root->dpy, &xi_maj, &xi_min) == BadRequest) return 1;

	int ndev;
	XIDeviceInfo *info = XIQueryDevice(root->dpy, XIAllMasterDevices, &ndev);
	for (int i = 0; i < ndev; i++) {
		if (vtk_root_update_xi2_scroll(root, info[i].classes, info[i].num_classes)) {
			root->xi2.device = info[i].deviceid;
			root->xi2.enable = true;
			break;
		}
	}
	if (!root->xi2.enable) return 1;

	memset(root->xi2.evt_mask_dat, 0, sizeof root->xi2.evt_mask_dat);
	root->xi2.emask = (XIEventMask){
		.deviceid = root->xi2.device,
		.mask_len = sizeof root->xi2.evt_mask_dat,
		.mask = root->xi2.evt_mask_dat,
	};
	XISetMask(root->xi2.emask.mask, XI_DeviceChanged);
	XISetMask(root->xi2.emask.mask, XI_Motion);
	XISetMask(root->xi2.emask.mask, XI_Enter);

	return 0;
}

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

	if (_vtk_root_xi2_init(*root)) {
		fprintf(stderr, "Error initialising XInput2. Falling back to legacy mouse button scrolling.\n");
	}

	return 0;
}

void vtk_destroy(vtk root) {
	XCloseDisplay(root->dpy);
	free(root);
}
