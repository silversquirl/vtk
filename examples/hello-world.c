#include <stdbool.h>
#include <stdio.h>
#include <cairo.h>
#include <vtk.h>

struct state {
	vtk_window win;
	cairo_t *cr;
	bool flip;
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

	if (s->flip) {
		cairo_move_to(s->cr, w, 0);
		cairo_line_to(s->cr, 0, h);
	} else {
		cairo_move_to(s->cr, 0, 0);
		cairo_line_to(s->cr, w, h);
	}
	cairo_set_source_rgb(s->cr, 1, 1, 1);
	cairo_set_line_width(s->cr, 2);
	cairo_stroke(s->cr);
}

void key_press_handler(vtk_event ev, void *u) {
	struct state *s = u;

	if (ev.key.key == VTK_K_ESCAPE) {
		close_handler(ev, u);
	} else if (ev.key.key == 'f' && ev.key.mods == VTK_M_CONTROL) {
		s->flip = !s->flip;
		vtk_window_redraw(s->win);
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
		.flip = false,
	};

	vtk_window_set_event_handler(win, VTK_EV_CLOSE, close_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_DRAW, draw_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_KEY_PRESS, key_press_handler, &s);

	vtk_window_mainloop(win);

	vtk_window_destroy(win);
	vtk_destroy(root);
	return 0;
}
