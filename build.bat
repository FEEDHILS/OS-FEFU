if not exist "./builddir" ( meson setup builddir )
meson compile -C builddir

.\builddir\app.exe