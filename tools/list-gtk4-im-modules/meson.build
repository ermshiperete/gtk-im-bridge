# Build list-gtk4-im-modules utility
if gtk4_dep.found()
  executable(
    'list-gtk4-im-modules',
    'list-gtk4-im-modules.c',
    dependencies: [gtk4_dep, glib_dep, gobject_dep, gio_dep],
    c_args: [
      '-g',
      '-Wall',
      '-Wextra',
    ],
  )
endif
