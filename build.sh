#!/bin/bash
git fetch -q && git pull -q

if [[ ! -d "./builddir" ]]; then
    meson setup builddir
fi

meson compile -C builddir
./builddir/app
