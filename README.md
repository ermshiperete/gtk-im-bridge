# GTK-IM-Bridge

A small GTK input module that bridges GTK's `GtkIMContext` to IBus and logs calls
to `/tmp/gtk-im-bridge.log`. Intended as a debugging tool to observe GTK ↔ IBus
interactions.

Supports both GTK3 and GTK4.

## Quick start

### Requirements

- meson, ninja
- `gtk4` or `gtk3` development packages
- `ibus-1.0` development packages

### Build

```bash
meson setup --prefix=/usr builddir
meson compile -C builddir
```

### Run the included runtime test (loads built module dynamically)

```bash
./builddir/tests/test-runtime-gtk4    # GTK4 test binary
./builddir/tests/test-runtime-gtk3    # GTK3 test binary
```

## Install (optional)

Install the module system-wide (may require sudo):

```bash
meson install -C builddir
# or copy manually:
# sudo cp builddir/gtk4/libim-bridge.so /usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules/
# sudo cp builddir/gtk3/im-bridge.so /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/
# sudo chmod 755 /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-bridge.so
# sudo /usr/lib/x86_64-linux-gnu/libgtk-3-0t64/gtk-query-immodules-3.0 --update-cache
```

## Usage

To run a app with this IM module:

```bash
GTK_IM_MODULE=im-bridge <gtk-application>
# Example:
# GTK_IM_MODULE=im-bridge builddir/simple-app-gtk4/simple-app-gtk4
# GTK_IM_MODULE=im-bridge IM_BRIDGE_MODULE=wayland builddir/simple-app-gtk3/simple-app-gtk3
```


## Logs

All function entry/exit and signal forwarding are logged to:

```
/tmp/gtk-im-bridge.log
```
