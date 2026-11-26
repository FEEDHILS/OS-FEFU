@echo off
git fetch -q && git pull -q

if not exist "./builddir" ( meson setup builddir )
meson compile -C builddir
if errorlevel 1 exit /b 1

".\builddir\app.exe"