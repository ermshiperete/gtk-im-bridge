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
