project('gtk-im-bridge', 'c', version: '0.1.0')

# Dependencies - common
ibus_dep = dependency('ibus-1.0', version: '>=1.5')
glib_dep = dependency('glib-2.0', version: '>=2.56')
gobject_dep = dependency('gobject-2.0', version: '>=2.56')
gio_dep = dependency('gio-2.0', version: '>=2.56')
gmodule_dep = dependency('gmodule-2.0', version: '>=2.56')

# GTK version selection
gtk_version = get_option('gtk-version')
if gtk_version == 'auto'
  gtk_dep = dependency('gtk4', version: '>=4.0', required: false)
  if gtk_dep.found()
    gtk_version = '4'
  else
    gtk_dep = dependency('gtk+-3.0', version: '>=3.0', required: true)
    gtk_version = '3'
  endif
else
  if gtk_version == '3'
    gtk_dep = dependency('gtk+-3.0', version: '>=3.0')
  else
    gtk_dep = dependency('gtk4', version: '>=4.0')
  endif
endif

# Subdirectories (version-specific demo apps)
if gtk_version == '4'
  subdir('simple-app-gtk4')
  subdir('tools')
else
  subdir('simple-app-gtk3')
endif

subdir('tests')

# Source files (shared across GTK versions)
shared_sources = [
  'src/logging.c',
  'src/im-bridge.c',
  'src/module.c',
]

headers = [
  'src/logging.h',
  'src/im-bridge.h',
]

# GTK4-specific: Get GTK module directory (install into GTK's versioned modules path)
# e.g. /usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules
if gtk_version == '4'
  gtk_libdir_opt = get_option('gtk_libdir')
  if gtk_libdir_opt != ''
    gtk_libdir = gtk_libdir_opt
  else
    gtk_libdir = join_paths(get_option('libdir'), 'gtk-4.0', '4.0.0', 'immodules')
  endif

  # Build shared library as GTK4 IM module
  im_module = shared_library(
    'im-bridge',
    shared_sources,
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

  message('im-bridge module (GTK4) will be installed to: ' + gtk_libdir)
else # GTK3
  gtk3_libdir_opt = get_option('gtk_libdir')
  if gtk3_libdir_opt != ''
    gtk3_libdir = gtk3_libdir_opt
  else
    gtk3_libdir = join_paths(get_option('libdir'), 'gtk-3.0', '3.0.0', 'immodules')
  endif

  # Build shared library as GTK3 IM module (without lib prefix)
  im_module = shared_library(
    'im-bridge',
    shared_sources,
    dependencies: [gtk_dep, ibus_dep, glib_dep, gobject_dep, gio_dep, gmodule_dep],
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
endif
