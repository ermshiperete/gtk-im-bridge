#include <stdio.h>
#include <gmodule.h>
#include <gtk/gtk.h>

typedef void (*im_module_init_t)(GTypeModule *);
typedef void (*im_module_exit_t)(void);
typedef GtkIMContext* (*im_module_create_t)(const char *);

int main(int argc, char **argv)
{
    gtk_init();

    const char *module_path = "../builddir/libim-bridge.so";
    GModule *mod = g_module_open(module_path, G_MODULE_BIND_LAZY);
    if (!mod) {
        g_printerr("Failed to open module: %s\n", g_module_error());
        return 1;
    }

    im_module_init_t im_init = NULL;
    im_module_create_t im_create = NULL;
    im_module_exit_t im_exit = NULL;

    if (!g_module_symbol(mod, "im_module_init", (gpointer *)&im_init)) {
        g_printerr("im_module_init not found: %s\n", g_module_error());
    }
    if (!g_module_symbol(mod, "im_module_create", (gpointer *)&im_create)) {
        g_printerr("im_module_create not found: %s\n", g_module_error());
    }
    if (!g_module_symbol(mod, "im_module_exit", (gpointer *)&im_exit)) {
        g_printerr("im_module_exit not found: %s\n", g_module_error());
    }

    if (im_init) im_init(NULL);
    GtkIMContext *ctx = NULL;
    if (im_create) ctx = im_create("test");

    if (ctx) {
        g_print("Created context: %p\n", (void*)ctx);
        gtk_im_context_focus_in(ctx);
        gtk_im_context_set_use_preedit(ctx, TRUE);

        char *pre = NULL;
        PangoAttrList *attrs = NULL;
        int cursor = 0;
        gtk_im_context_get_preedit_string(ctx, &pre, &attrs, &cursor);
        g_print("Preedit: '%s' cursor=%d\n", pre ? pre : "", cursor);
        g_clear_object(&attrs);
        g_free(pre);

        gtk_im_context_reset(ctx);
        gtk_im_context_focus_out(ctx);

        g_object_unref(ctx);
    }

    if (im_exit) im_exit();
    g_module_close(mod);

    return 0;
}
