#include <stdio.h>
#include <cairo.h>
#include <vtk.h>

struct state {
	vtk_window win;
	cairo_t *cr;
	int sx, sy, ex, ey;
};

void close_handler(vtk_event ev, void *u) {
	struct state *s = u;
	vtk_window_close(s->win);
}

void draw_handler(vtk_event ev, void *u) {
	struct state *s = u;

	int w, h;
	vtk_window_get_size(s->win, &w, &h);

	cairo_rectangle(s->cr, 0, 0, w, h);
	cairo_set_source_rgb(s->cr, 0, 0, 0);
	cairo_fill(s->cr);

	cairo_move_to(s->cr, s->sx, s->sy);
	cairo_line_to(s->cr, s->ex, s->ey);
	cairo_set_source_rgb(s->cr, 1, 1, 1);
	cairo_set_line_width(s->cr, 2);
	cairo_stroke(s->cr);
}

void mouse_move_handler(vtk_event ev, void *u) {
	struct state *s = u;
	if (ev.mouse_move.mods & VTK_M_LEFT_BTN) {
		s->ex = ev.mouse_move.x;
		s->ey = ev.mouse_move.y;
		vtk_window_redraw(s->win);
	}
}

void mouse_press_handler(vtk_event ev, void *u) {
	struct state *s = u;
	if (ev.mouse_button.btn == VTK_M_LEFT_BTN) {
		s->sx = ev.mouse_button.x;
		s->sy = ev.mouse_button.y;
	}
}

int main() {
	int err;

	vtk root;
	if ((err = vtk_new(&root))) {
		fprintf(stderr, "Error initialising vtk root: %s\n", vtk_strerr(err));
		return 1;
	}

	vtk_window win;
	if ((err = vtk_window_new(&win, root, "Hello, world!", 0, 0, 800, 600))) {
		fprintf(stderr, "Error initialising vtk root: %s\n", vtk_strerr(err));
		vtk_destroy(root);
		return 1;
	}

	struct state s = {
		.win = win,
		.cr = vtk_window_get_cairo(win),
		.sx = 0,
		.sy = 0,
		.ex = 0,
		.ey = 0,
	};

	vtk_window_set_event_handler(win, VTK_EV_CLOSE, close_handler);
	vtk_window_set_event_handler(win, VTK_EV_DRAW, draw_handler);
	vtk_window_set_event_handler(win, VTK_EV_MOUSE_MOVE, mouse_move_handler);
	vtk_window_set_event_handler(win, VTK_EV_MOUSE_PRESS, mouse_press_handler);
	vtk_window_set_event_handler_data(win, &s);

	vtk_window_mainloop(win);

	vtk_window_destroy(win);
	vtk_destroy(root);
	return 0;
}
