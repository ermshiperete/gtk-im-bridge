if gtk4_dep.found()
  # Build test executable (without module.c to avoid GIO module loading)
  test_runtime = executable(
    'test-runtime-gtk4',
    'test_runtime.c',
    '../src/im-bridge.c',
    '../src/logging.c',
    dependencies: [gtk4_dep, ibus_dep, glib_dep, gobject_dep],
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

if gtk3_dep.found()
  gmodule_dep = dependency('gmodule-2.0', required: false)

  if gmodule_dep.found()
    test_dynamic_load = executable(
      'test-dynamic-load-gtk3',
      'test-dynamic-load.c',
      dependencies: [gtk3_dep, gmodule_dep],
      install: false
    )

    test('dynamic-load', test_dynamic_load)
  endif

  # Build test executable
  test_runtime_gtk3 = executable(
    'test-runtime-gtk3',
    'test_runtime.c',
    '../src/im-bridge.c',
    '../src/logging.c',
    dependencies: [gtk3_dep, ibus_dep, glib_dep, gobject_dep, gio_dep, gmodule_dep],
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
