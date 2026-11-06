project(
  'Hello world',
  'c',
  meson_version : '>= 1.3.0',
  version : '0.1',
  default_options : ['warning_level=3'],
)

dependencies = [
]

sources = [
  './src/main.c',
]

if host_machine.system() == 'windows'
    sources += './src/utils_win.c'
elif host_machine.system() == 'linux'
    sources += './src/utils_linux.c'
endif

executable(
  'app',
  sources,
  dependencies : dependencies,
  install : true,
)
