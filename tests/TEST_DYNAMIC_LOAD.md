# Dynamic IM Module Loading Test

## Overview

The `test-dynamic-load` executable tests the dynamic loading of GTK3 IM
modules (specifically `im-ibus.so`) and invokes the module's
initialization and context creation functions.

## Purpose

This test was created to isolate and diagnose issues with the
`_create_context_instance()` function in `im-bridge.c` crashing when
calling `im_module_init()` and `im_module_create()`.

## Build

The test is automatically built with the meson build system when GTK3 is
selected:

```bash
meson setup builddir -Dgtk-version=3
meson compile -C builddir
```

## Run

```bash
./builddir/tests/test-dynamic-load
```

## What It Does

1. **Initializes GTK** - Calls `gtk_init()` to set up GTK context
2. **Dynamically loads im-ibus.so** - Uses `g_module_open()` to load the
   shared library
3. **Retrieves function pointers** - Gets `im_module_init()` and
   `im_module_create()` symbols
4. **Creates a test GTypeModule** - Implements a minimal `GTypeModule`
   subclass for testing
5. **Calls im_module_init()** - Initializes the ibus module with the
   GTypeModule
6. **Calls im_module_create()** - Creates an IBusIMContext instance
7. **Reports results** - Shows the context type and address if
   successful

## Key Findings

✅ **Dynamic loading works correctly** - The module is loaded and symbols
are found ✅ **im_module_init() succeeds** - When given a proper
GTypeModule instance ✅ **im_module_create() succeeds** - Returns a valid
IBusIMContext object

### Sample Output

```
=== GTK IM Module Dynamic Load Test ===
GTK version: 3.24.41

Attempting to load im-ibus module...
Failed with im-ibus.so, trying /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-ibus.so...
Module loaded successfully: /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-ibus.so

Loading im_module_init...
im_module_init loaded at 0x79cfb068eac0
Loading im_module_create...
im_module_create loaded at 0x79cfb068eeb0

Calling im_module_init...
TestTypeModule created at 0x57b3cc12da30
Calling im_module_init with TestTypeModule...
test_type_module_load called
im_module_init completed successfully

Calling im_module_create('ibus')...
SUCCESS: im_module_create returned context at 0x57b3cc1f2650
Context type: IBusIMContext

=== Test completed ===
```

## Debugging Notes

The test uses a minimal `TestTypeModule` GObject subclass to properly
satisfy the GTK3 IM module API requirements. The key insight is that
`im_module_init()` requires a valid `GTypeModule*` parameter - passing
NULL or an invalid pointer causes the ibus module to fail with assertion
errors.

When comparing with `_create_context_instance()` in `im-bridge.c` (GTK3
version, line 517):
- The code retrieves `im_module` from `self->priv->im_module` (a
  GModule*)
- It then calls `im_module_init(G_TYPE_MODULE(self->priv->im_module))`

The issue is that `self->priv->im_module` is a `GModule*`, not a
`GTypeModule*`. The code incorrectly casts a pointer to a loaded dynamic
library to a GType object, which is invalid and causes the crash.

## Implications

The GTK3 implementation of `_create_context_instance()` needs to:
1. Create a proper `GTypeModule` instance
2. Pass that to `im_module_init()` instead of casting the `GModule*`

This test demonstrates that the ibus module itself works correctly when
called properly.
