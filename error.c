#include "vtk.h"

char *vtk_strerr(vtk_err err) {
	switch (err) {
	case VTK_SUCCESS:
		return "Success";
	case VTK_ALLOCATION_FAILED:
		return "Allocation failed";
	case VTK_NO_SUITABLE_VISUAL:
		return "Could not find TrueColor visual for X display";
	case VTK_XOPENDISPLAY_FAILED:
		return "Opening X display failed";
	default:
		return "Unknown error";
	}
}
