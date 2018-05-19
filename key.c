#include <strings.h>
#include <ctype.h>
#include <vtk.h>

inline static int len1(const char *s) {
	return *s != '\0' && s[1] == '\0'; // Is the string 1 char long?
}

vtk_key vtk_key_from_string(const char *s) {
	if (!strcasecmp(s, "BACKSPACE")) return VTK_K_BACKSPACE;
	else if (!strcasecmp(s, "TAB")) return VTK_K_TAB;
	else if (!strcasecmp(s, "RETURN")) return VTK_K_RETURN;
	else if (!strcasecmp(s, "ESCAPE")) return VTK_K_ESCAPE;
	else if (!strcasecmp(s, "SPACE")) return VTK_K_SPACE;
	else if (!strcasecmp(s, "DELETE")) return VTK_K_DELETE;
	else if (!strcasecmp(s, "INSERT")) return VTK_K_INSERT;

	else if (!strcasecmp(s, "PAGE_UP")) return VTK_K_PAGE_UP;
	else if (!strcasecmp(s, "PAGEUP")) return VTK_K_PAGE_UP;
	else if (!strcasecmp(s, "PGUP")) return VTK_K_PAGE_UP;
	else if (!strcasecmp(s, "PAGE_DOWN")) return VTK_K_PAGE_DOWN;
	else if (!strcasecmp(s, "PAGEDOWN")) return VTK_K_PAGE_DOWN;
	else if (!strcasecmp(s, "PGDN")) return VTK_K_PAGE_DOWN;
	else if (!strcasecmp(s, "HOME")) return VTK_K_HOME;
	else if (!strcasecmp(s, "END")) return VTK_K_END;
	else if (!strcasecmp(s, "UP")) return VTK_K_UP;
	else if (!strcasecmp(s, "DOWN")) return VTK_K_DOWN;
	else if (!strcasecmp(s, "LEFT")) return VTK_K_LEFT;
	else if (!strcasecmp(s, "RIGHT")) return VTK_K_RIGHT;

	// Literal characters
	else if (len1(s) && ' ' <= *s && *s <= '~') return *s;

	return VTK_K_NONE;
}

vtk_modifiers vtk_modifier_from_string(const char *s) {
	if (!strcasecmp(s, "SHIFT")) return VTK_M_SHIFT;
	else if (!strcasecmp(s, "CAPS_LOCK")) return VTK_M_CAPS_LOCK;
	else if (!strcasecmp(s, "CAPSLOCK")) return VTK_M_CAPS_LOCK;
	else if (!strcasecmp(s, "CONTROL")) return VTK_M_CONTROL;
	else if (!strcasecmp(s, "CTRL")) return VTK_M_CONTROL;
	else if (!strcasecmp(s, "ALT")) return VTK_M_ALT;
	else if (!strcasecmp(s, "SUPER")) return VTK_M_SUPER;

	else if (!strcasecmp(s, "LEFT_BTN")) return VTK_M_LEFT_BTN;
	else if (!strcasecmp(s, "LEFT")) return VTK_M_LEFT_BTN;
	else if (!strcasecmp(s, "BTN1")) return VTK_M_LEFT_BTN;
	else if (!strcasecmp(s, "MIDDLE_BTN")) return VTK_M_MIDDLE_BTN;
	else if (!strcasecmp(s, "MIDDLE")) return VTK_M_MIDDLE_BTN;
	else if (!strcasecmp(s, "BTN2")) return VTK_M_LEFT_BTN;
	else if (!strcasecmp(s, "RIGHT_BTN")) return VTK_M_RIGHT_BTN;
	else if (!strcasecmp(s, "RIGHT")) return VTK_M_RIGHT_BTN;
	else if (!strcasecmp(s, "BTN3")) return VTK_M_LEFT_BTN;

	// Special shorter versions
	if (len1(s)) {
		switch (toupper(*s)) {
		case 'S': return VTK_M_SHIFT;
		case 'C': return VTK_M_CONTROL;
		case 'A': return VTK_M_ALT;
		}
	}

	return VTK_K_NONE;
}
