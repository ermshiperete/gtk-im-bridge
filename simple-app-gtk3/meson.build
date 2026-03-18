if gtk3_dep.found()
  executable(
    'simple-app-gtk3',
    'simple-app-gtk3.c',
    dependencies: [gtk3_dep],
    c_args: [
      '-g',
      '-Wall',
      '-Wextra',
      '-Wno-unused-parameter',
    ],
  )
endif
