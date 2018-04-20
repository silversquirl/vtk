#include <stdio.h>
#include <cairo.h>
#include <vtk.h>

struct state {
	vtk_window win;
	cairo_t *cr;
	int x, y;
};

void close_handler(vtk_event ev, void *u) {
	struct state *s = u;
	vtk_window_close(s->win);
}

void draw_handler(vtk_event ev, void *u) {
	struct state *s = u;

	int w, h;
	vtk_window_get_size(s->win, &w, &h);

	cairo_pattern_t *g = cairo_pattern_create_radial(s->x, s->y, 0, s->x, s->y, 200);
	cairo_pattern_add_color_stop_rgba(g, 0, .4, 1, .4, 1);
	cairo_pattern_add_color_stop_rgba(g, .3, 0, 1, 0, .5);
	cairo_pattern_add_color_stop_rgba(g, 1, 0, 1, 0, 0);

	cairo_rectangle(s->cr, 0, 0, w, h);
	cairo_set_source(s->cr, g);
	cairo_fill(s->cr);
}

void mouse_move_handler(vtk_event ev, void *u) {
	struct state *s = u;
	s->x = ev.mouse_move.x;
	s->y = ev.mouse_move.y;
	vtk_window_redraw(s->win);
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
		.x = 0,
		.y = 0,
	};

	vtk_window_set_event_handler(win, VTK_EV_CLOSE, close_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_DRAW, draw_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_MOUSE_MOVE, mouse_move_handler, &s);

	vtk_window_mainloop(win);

	vtk_window_destroy(win);
	vtk_destroy(root);
	return 0;
}
