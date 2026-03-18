# GTK3 IM Module Build

gtk3_libdir_opt = get_option('gtk_libdir')
if gtk3_libdir_opt != ''
  gtk3_libdir = gtk3_libdir_opt
else
  gtk3_libdir = join_paths(get_option('libdir'), 'gtk-3.0', '3.0.0', 'immodules')
endif

# Build shared library as GTK3 IM module (without lib prefix)
im_module_gtk3 = shared_library(
  'im-bridge',
  '../src/logging.c',
  '../src/im-bridge.c',
  '../src/module.c',
  dependencies: [gtk3_dep, ibus_dep, glib_dep, gobject_dep, gio_dep, gmodule_dep],
  install: true,
  install_dir: gtk3_libdir,
  name_prefix: '',
  c_args: [
    '-g',
    '-Wall',
    '-Wextra',
    '-Wno-unused-parameter',
  ],
)

message('im-bridge module (GTK3) will be installed to: ' + gtk3_libdir)
