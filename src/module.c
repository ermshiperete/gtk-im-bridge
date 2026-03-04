#include <gtk/gtk.h>
#include <gmodule.h>
#include <gio/gio.h>
#include "im-bridge.h"
#include "logging.h"
#include "ibus-setup.h"

/* Module entry points that GTK looks for */

void
im_module_init(GTypeModule *module)
{
  LOG_ENTER("im_module_init", "module=%p", (void *)module);
  
  logging_init();
  
  /* Ensure the type is registered (will be auto-registered by G_DEFINE_FINAL_TYPE) */
  gtk_im_bridge_context_get_type();
  
  LOG_EXIT("im_module_init", "");
}

void
im_module_exit(void)
{
  LOG_ENTER("im_module_exit", "");
  
  ibus_setup_cleanup();
  
  LOG_EXIT("im_module_exit", "");
  LOG_SIGNAL("gtk-im-bridge-shutdown", "module exit complete");
}

GtkIMContext *
im_module_create(const char *context_id)
{
  LOG_ENTER("im_module_create", "context_id='%s'", context_id ? context_id : "");
  
  GtkIMContext *context = gtk_im_bridge_context_new();
  
  LOG_EXIT("im_module_create", "context=%p", (void *)context);
  return context;
}

/* GIO-style module entry points expected by GTK's module loader. */

G_MODULE_EXPORT void
g_io_module_load(GIOModule *module)
{
  g_type_module_use(G_TYPE_MODULE(module));

  /* If this were a G_DEFINE_DYNAMIC_TYPE we'd call the generated
   * register function here (e.g. gtk_im_bridge_context_register_type
   * (G_TYPE_MODULE (module))). We still ensure the type is known by
   * calling the get_type helper and initialize module state.
   */
  gtk_im_bridge_context_get_type();

  /* Register the extension point implementation: name, required type,
   * extension name, priority. */
  g_io_extension_point_implement("gtk-im-module",
                                GTK_TYPE_IM_CONTEXT,
                                "im-bridge",
                                10);

  im_module_init(G_TYPE_MODULE(module));
}

G_MODULE_EXPORT void
g_io_module_unload(GIOModule *module)
{
  im_module_exit();
  g_type_module_unuse(G_TYPE_MODULE(module));
}

G_MODULE_EXPORT char **
g_io_module_query(void)
{
  static char *eps[] = { "gtk-im-module", NULL };
  return g_strdupv(eps);
}
