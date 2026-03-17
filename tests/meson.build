if gtk_version == '4'
  # Build test executable (without module.c to avoid GIO module loading)
  test_runtime = executable(
    'test_runtime',
    'test_runtime.c',
    '../src/im-bridge.c',
    '../src/logging.c',
    dependencies: [gtk_dep, ibus_dep, glib_dep, gobject_dep, gio_dep],
    include_directories: include_directories('../src'),
    c_args: [
      '-g',
      '-Wall',
      '-Wextra',
      '-Wno-unused-parameter',
      '-D_BUILD_TEST',
    ],
  )
else
  gtk_dep = dependency('gtk+-3.0', required: false)
  gmodule_dep = dependency('gmodule-2.0', required: false)

  if gtk_dep.found() and gmodule_dep.found()
    test_dynamic_load = executable(
      'test-dynamic-load',
      'test-dynamic-load.c',
      dependencies: [gtk_dep, gmodule_dep],
      install: false
    )

    test('dynamic-load', test_dynamic_load)
  endif

  # Build test executable
  test_runtime = executable(
    'test_runtime',
    'test_runtime.c',
    '../src/im-bridge.c',
    '../src/logging.c',
    dependencies: [gtk_dep, ibus_dep, glib_dep, gobject_dep, gio_dep, gmodule_dep],
    include_directories: include_directories('../src'),
    c_args: [
      '-g',
      '-Wall',
      '-Wextra',
      '-Wno-unused-parameter',
      '-D_BUILD_TEST',
    ],
  )
endif
