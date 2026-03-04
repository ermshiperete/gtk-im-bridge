**GTK-IM-Bridge — Implementation Plan**

- **Purpose**: Implement a GTK input module (`gtk-im-module`) that bridges GTK's `GtkIMContext` to IBus and logs every implemented function call (name, parameters, return values) to `/tmp/gtk-im-bridge.log`. This is intended as a debugging tool to inspect GTK↔IBus interactions.

- **Language**: C
- **Build system**: Meson (project already configured)
- **Target platform**: Ubuntu 24.04 (GTK4 + IBus available)

**Work breakdown**

1. Project scaffolding
   - Meson files and options
2. Logging
   - Centralized logging API that writes to `/tmp/gtk-im-bridge.log` with timestamps
3. IBus connection management
   - Initialize IBus, create and cache an `IBusInputContext`, handle cleanup
4. `GtkIMContext` wrapper implementation
   - Create `GtkImBridgeContext` deriving from `GtkIMContext`
   - Implement virtual methods and forward to IBus when appropriate
   - Proxy relevant IBus signals to GTK signals
5. Module entry points
   - Provide `im_module_init`, `im_module_exit`, `im_module_create`
6. Build and test
   - Compile and verify the shared library `libim-bridge.so`
   - Install to GTK IM module directory (requires appropriate privileges)
7. Iteration
   - Expand logging detail, convert IBus attributes to Pango attributes if needed

**Files added so far**
- `meson.build`, `meson_options.build`
- `src/logging.h`, `src/logging.c`
- `src/ibus-setup.h`, `src/ibus-setup.c`
- `src/im-bridge.h`, `src/im-bridge.c`
- `src/module.c`

**Current status**
- Implemented the core bridge and logging.
- Project builds successfully; shared library produced at `builddir/libim-bridge.so`.

**Build / Install / Test**

To build (already done during implementation):

```bash
meson setup builddir
meson compile -C builddir
```

To install the module system-wide (may require sudo):

```bash
meson install -C builddir
# or copy the built library to your GTK module directory manually
# sudo cp builddir/libim-bridge.so /usr/lib/gtk-4.0/immodules/libim-bridge.so
```

To test a GTK4 application with the bridge enabled (example):

```bash
GTK_IM_MODULE=im-bridge /usr/bin/gtk4-demo
# or
GTK_IM_MODULE=im-bridge gedit
```

Check logs at:

```
/tmp/gtk-im-bridge.log
```

**Next steps**
- (Optional) Improve attribute translation from IBus attributes to Pango attributes for accurate preedit rendering.
- Add unit/integration tests or a small GTK test app that demonstrates preedit and commit flows.
- Optionally add Meson `install_data` to install a `gtk-query-immodules` metadata file if needed by the platform.

If you want, I can now:
- Install the module to the system GTK IM modules directory, or
- Run a short runtime test with `gtk4-demo` and capture the first log entries, or
- Implement conversion of IBus attributes to Pango attributes.
