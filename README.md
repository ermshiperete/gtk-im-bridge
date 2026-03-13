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

#### For GTK4 (default):

```bash
meson setup --prefix=/usr builddir/gtk4
meson compile -C builddir/gtk4
```

#### For GTK3:

```bash
meson setup --prefix=/usr -Dgtk-version=3 builddir/gtk3
meson compile -C builddir/gtk3
```

Auto-detection (tries GTK4 first, falls back to GTK3):

```bash
meson setup --prefix=/usr builddir/gtk4  # uses -Dgtk-version=auto (default)
```

### Run the included runtime test (loads built module dynamically)

```bash
./builddir/gtk4/test_runtime    # GTK4 test binary
./builddir/gtk3/test_runtime    # GTK3 test binary
```

## Install (optional)

### GTK4

Install the module system-wide (may require sudo):

```bash
meson install -C builddir/gtk4
# or copy manually:
# sudo cp builddir/gtk4/libim-bridge.so /usr/lib/x86_64-linux-gnu/gtk-4.0/4.0.0/immodules/
```

### GTK3

```bash
meson install -C builddir/gtk3
# or copy manually:
# sudo cp builddir/gtk3/im-bridge.so /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/
# sudo chmod 755 /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-bridge.so
# sudo /usr/lib/x86_64-linux-gnu/libgtk-3-0t64/gtk-query-immodules-3.0 --update-cache
```

## Usage

To run a GTK4 app with this IM module:

```bash
GTK_IM_MODULE=im-bridge <gtk4-application>
# Example:
# GTK_IM_MODULE=im-bridge builddir/gtk4/simple-app-gtk4/simple-app-gtk4
```

To run a GTK3 app with this IM module:

```bash
GTK_IM_MODULE=im-bridge <gtk3-application>
# Example:
# GTK_IM_MODULE=im-bridge builddir/gtk3/simple-app-gtk3/simple-app-gtk3
```

## Logs

All function entry/exit and signal forwarding are logged to:

```
/tmp/gtk-im-bridge.log
```
