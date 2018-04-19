# vtk

A GUI "toolkit" by vktec

## What?

vtk is my take on a GUI toolkit. It's incredibly minimal and adds very
little on top of the libraries it wraps, preferring to simply leave the
implementation details up to the user. It has no widgets of any kind,
and instead allows you to draw whatever you want using Cairo, including
high-quality text through PangoCairo.

In summary, vtk is a system for handling events and creating a Cairo
context that will draw to the screen.

Currently, vtk only has an Xlib backend, but it could in theory be
ported to other systems by using Cairo's Quartz and Win32 backends,
or even to the Linux framebuffer by using Cairo's image surfaces.

## Why?

There are plenty of pre-existing GUI toolkits, why create another?

My opinion on GUI toolkits is that they overcomplicate things. Most of
them are huge, monolithic libraries that try to do everything at once:
event handling, graphics, widgets, text, internationalization, command
line argument parsing, IO, networking, etc.

vtk takes the opposite approach, aiming to do as *little* as possible
while still being useful. In other words, it aims to be a [minimal
viable program][mvp].

[mvp]: https://joearms.github.io/published/2014-06-25-minimal-viable-program.html
