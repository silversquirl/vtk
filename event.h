#ifndef __VTK_EVENT_H__
#define __VTK_EVENT_H__

#include <X11/Xlib.h>
#include "vtk.h"

void vtk_event_handle(vtk_window win, XEvent ev);

#endif
