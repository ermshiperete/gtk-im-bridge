# GTK3 GTypeModule Fix

## Problem

The GTK3 version of `_create_context_instance()` was crashing when calling `im_module_init()` with an invalid `GTypeModule*` parameter.

The original code attempted to cast the `GModule*` pointer directly to `GTypeModule*`:
```c
im_module_init(G_TYPE_MODULE(self->priv->im_module));  // WRONG!
```

This is invalid because:
1. `GModule*` is a GLib opaque pointer representing a loaded dynamic library
2. `GTypeModule*` is a GObject type that requires proper initialization and lifecycle management
3. You cannot simply cast between these types - the memory layout is incompatible

## Solution

Created a proper `GTypeModule` wrapper class called `ImModuleWrapper` that:

1. **Wraps the GModule**: Stores the `GModule*` pointer in a new struct:
   ```c
   typedef struct {
     GTypeModule parent;
     GModule *gmodule;
   } ImModuleWrapper;
   ```

2. **Implements GTypeModule interface**: Provides `load()` and `unload()` methods that are called by the GLib type system:
   ```c
   static gboolean im_module_wrapper_load(GTypeModule *module) {
     return TRUE;  // GModule is already loaded
   }

   static void im_module_wrapper_unload(GTypeModule *module) {
     // Don't unload GModule - it's managed separately
   }
   ```

3. **Proper lifecycle management**: 
   - Store the wrapper in `self->priv->im_module_wrapper`
   - Never unref the wrapper after types are registered (GTypeModule has special disposal rules)
   - The wrapper remains in memory for the process lifetime
   - We just set it to NULL during finalize (without unreffing)

## Key Implementation Details

### Creating the Wrapper
```c
GTypeModule *gtype_module = g_object_new(im_module_wrapper_get_type(), NULL);
ImModuleWrapper *wrapper = (ImModuleWrapper *)gtype_module;
wrapper->gmodule = self->priv->im_module;

// Store it to keep it alive
self->priv->im_module_wrapper = gtype_module;

// Now safe to call with proper GTypeModule
im_module_init(gtype_module);
```

### Finalize Behavior
```c
// Don't unref! GTypeModule has special lifecycle management.
// Once types are registered, the GTypeModule must NOT be disposed.
self->priv->im_module_wrapper = NULL;
```

## Why We Don't Unref GTypeModule

The GObject library has special handling for `GTypeModule` objects. When you register a GType with a GTypeModule:

1. The GTypeModule takes a reference to ensure it stays alive
2. The type system tracks which GTypeModule owns each registered type
3. Disposing a GTypeModule with registered types causes assertion failures
4. GTypeModules are meant to live for the lifetime of the module that registered types with them

Therefore, once we call `im_module_init()` which registers types with our wrapper, we must:
- **NOT** call `g_object_unref()` on the wrapper
- **NOT** try to dispose it normally
- Let it remain in memory for the process lifetime
- Accept the small memory leak (one GObject per IM context instance)

This is the standard pattern when using GTypeModule for dynamically loaded modules.

## Testing

The fix was validated with:
- `tests/test_runtime` - Creates an IM context, uses it, and exits cleanly
- `tests/test-dynamic-load` - Directly tests dynamic module loading with ImModuleWrapper
- No assertions or warnings from GLib during normal operation

All tests pass without the "unsolicitated invocation of g_object_run_dispose()" warnings that appeared before the fix.
