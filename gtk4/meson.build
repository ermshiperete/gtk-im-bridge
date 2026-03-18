# GTK4 IM Module Build

gtk4_libdir_opt = get_option('gtk_libdir')
if gtk4_libdir_opt != ''
  gtk4_libdir = gtk4_libdir_opt
else
  gtk4_libdir = join_paths(get_option('libdir'), 'gtk-4.0', '4.0.0', 'immodules')
endif

# Build shared library as GTK4 IM module
im_module_gtk4 = shared_library(
  'im-bridge',
  '../src/logging.c',
  '../src/im-bridge.c',
  '../src/module.c',
  dependencies: [gtk4_dep, ibus_dep, glib_dep, gobject_dep, gio_dep],
  install: true,
  install_dir: gtk4_libdir,
  c_args: [
    '-g',
    '-Wall',
    '-Wextra',
    '-Wno-unused-parameter',
  ],
)

message('im-bridge module (GTK4) will be installed to: ' + gtk4_libdir)
