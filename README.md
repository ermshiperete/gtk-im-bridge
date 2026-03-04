# GTK-IM-Bridge

A small GTK input module that bridges GTK's `GtkIMContext` to IBus and logs calls
to `/tmp/gtk-im-bridge.log`. Intended as a debugging tool to observe GTK ↔ IBus
interactions.

## Quick start

### Requirements

- meson, ninja
- `gtk4` development packages
- `ibus-1.0` development packages

### Build

```bash
meson setup builddir
meson compile -C builddir
```

### Run the included runtime test (loads built module dynamically)

```bash
./builddir/test_runtime    # built test binary (if present)
# or compile and run the test manually:
# cc `pkg-config --cflags gtk4 ibus-1.0 gmodule-2.0` tests/test_runtime.c -o builddir/test_runtime `pkg-config --libs gtk4 ibus-1.0 gmodule-2.0`
# ./builddir/test_runtime
```

## Install (optional)

Install the module system-wide (may require sudo):

```bash
meson install -C builddir
# or copy manually:
# sudo cp builddir/libim-bridge.so /usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules/
```

## Usage

To run a GTK4 app with this IM module:

```bash
GTK_IM_MODULE=im-bridge <gtk4-application>
# Example:
# GTK_IM_MODULE=im-bridge gtk4-demo
```

## Logs

All function entry/exit and signal forwarding are logged to:

```
/tmp/gtk-im-bridge.log
```

## Files added

- `meson.build`, `meson_options.build`
- `src/` — implementation and logging
- `tests/test_runtime.c` — small dynamic loader test
- `PLAN.md` — short implementation plan

## Next steps

- Install the module and test in real applications, or I can commit and install it for you.

## License

No license file included; add one if you plan to publish.
