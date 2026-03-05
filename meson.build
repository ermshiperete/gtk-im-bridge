project('gtk-im-bridge', 'c', version: '0.1.0')

# Dependencies
gtk_dep = dependency('gtk4', version: '>=4.0')
ibus_dep = dependency('ibus-1.0', version: '>=1.5')
glib_dep = dependency('glib-2.0', version: '>=2.56')
gobject_dep = dependency('gobject-2.0', version: '>=2.56')
gio_dep = dependency('gio-2.0', version: '>=2.56')

# Get GTK module directory (install into GTK's versioned modules path)
# e.g. /usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules
gtk_libdir_opt = get_option('gtk_libdir')
if gtk_libdir_opt != ''
  gtk_libdir = gtk_libdir_opt
else
  gtk_libdir = join_paths(get_option('libdir'), 'gtk-4.0', '4.0.0', 'immodules')
endif

# Source files
sources = [
  'src/logging.c',
  'src/ibus-setup.c',
  'src/im-bridge.c',
  'src/module.c',
]

headers = [
  'src/logging.h',
  'src/ibus-setup.h',
  'src/im-bridge.h',
]

# Build shared library as GTK IM module
im_module = shared_library(
  'im-bridge',
  sources,
  dependencies: [gtk_dep, ibus_dep, glib_dep, gobject_dep, gio_dep],
  install: true,
  install_dir: gtk_libdir,
  c_args: [
    '-g',
    '-Wall',
    '-Wextra',
    '-Wno-unused-parameter',
  ],
)
