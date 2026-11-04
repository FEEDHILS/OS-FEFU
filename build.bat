@echo off
git fetch -q && git pull -q

if not exist "./builddir" ( meson setup builddir )
meson compile -C builddir

.\builddir\app.exe