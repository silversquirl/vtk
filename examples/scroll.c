#define _DEFAULT_SOURCE
#include <math.h>
#include <stdio.h>
#include <cairo.h>
#include <vtk.h>

struct state {
	vtk_window win;
	cairo_t *cr;
	double angle;
};

void close_handler(vtk_event ev, void *u) {
	struct state *s = u;
	vtk_window_close(s->win);
}

void draw_handler(vtk_event ev, void *u) {
	struct state *s = u;

	int w, h;
	vtk_window_get_size(s->win, &w, &h);

	cairo_translate(s->cr, 0, 0);
	cairo_rotate(s->cr, 0);
	cairo_rectangle(s->cr, 0, 0, w, h);
	cairo_set_source_rgb(s->cr, 0, 0, 0);
	cairo_fill(s->cr);

	cairo_translate(s->cr, w/2, h/2);
	cairo_rotate(s->cr, s->angle);
	cairo_move_to(s->cr, -w/2, -h/2);
	cairo_line_to(s->cr, w/2, h/2);
	cairo_set_source_rgb(s->cr, 1, 1, 1);
	cairo_set_line_width(s->cr, 2);
	cairo_stroke(s->cr);
}

void scroll_handler(vtk_event ev, void *u) {
	struct state *s = u;
	s->angle -= ev.scroll.amount / (8 * M_PI);
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
		.angle = 0.0,
	};

	vtk_window_set_event_handler(win, VTK_EV_CLOSE, close_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_DRAW, draw_handler, &s);
	vtk_window_set_event_handler(win, VTK_EV_SCROLL, scroll_handler, &s);

	vtk_window_mainloop(win);

	vtk_window_destroy(win);
	vtk_destroy(root);
	return 0;
}
