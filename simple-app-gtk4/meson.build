if gtk4_dep.found()
  executable(
    'simple-app-gtk4',
    'simple-app-gtk4.c',
    dependencies: [gtk4_dep],
    c_args: [
      '-g',
      '-Wall',
      '-Wextra',
      '-Wno-unused-parameter',
    ],
  )
endif
