project('gtk-im-bridge', 'c', version: '0.1.0')

# Dependencies - common
ibus_dep = dependency('ibus-1.0', version: '>=1.5')
glib_dep = dependency('glib-2.0', version: '>=2.56')
gobject_dep = dependency('gobject-2.0', version: '>=2.56')
gio_dep = dependency('gio-2.0', version: '>=2.56')
gmodule_dep = dependency('gmodule-2.0', version: '>=2.56')

# Source files (shared across GTK versions)
shared_sources = [
  'src/logging.c',
  'src/im-bridge.c',
  # 'src/im-bridge-minimal.c',
  'src/module.c',
]

headers = [
  'src/logging.h',
  'src/im-bridge.h',
]

# Common compiler flags
common_c_args = [
  '-g',
  '-Wall',
  '-Wextra',
  '-Wno-unused-parameter',
]

# Check for GTK4
gtk4_dep = dependency('gtk4', version: '>=4.0', required: false)
gtk3_dep = dependency('gtk+-3.0', version: '>=3.0', required: false)

# Error if neither GTK version is available
if not gtk4_dep.found() and not gtk3_dep.found()
  error('Neither GTK3 nor GTK4 development packages found')
endif

# Build GTK4 in gtk4 subdirectory if available
if gtk4_dep.found()
  subdir('gtk4')
endif

# Build GTK3 in gtk3 subdirectory if available  
if gtk3_dep.found()
  subdir('gtk3')
endif

# Subdirectories for apps and tests
subdir('simple-app-gtk4')
subdir('simple-app-gtk3')
subdir('tools')
subdir('tests')
