#include <gtk/gtk.h>
#include <gmodule.h>
#include <gio/gio.h>
#include "im-bridge.h"
#include "logging.h"

/* Module entry points that GTK looks for */

G_MODULE_EXPORT void
im_module_init(GTypeModule *module)
{
  g_type_module_use(G_TYPE_MODULE(module));
  logging_init();

  LOG_ENTER("im_module_init", "module=%p", (void *)module);
  LOG_EXIT("im_module_init", "");
}

G_MODULE_EXPORT void
im_module_exit(void)
{
  LOG_ENTER("im_module_exit", "");

  LOG_EXIT("im_module_exit", "");
  LOG_SIGNAL("gtk-im-bridge-shutdown", "module exit complete");
}

G_MODULE_EXPORT GtkIMContext *
im_module_create(const char *context_id)
{
  GtkIMContext *context;

  LOG_ENTER("im_module_create", "context_id='%s'", context_id ? context_id : "");

  context = gtk_im_bridge_context_new();

  LOG_EXIT("im_module_create", "context=%p", (void *)context);
  return context;
}

#if !GTK_CHECK_VERSION(4, 0, 0)
/* GTK3: Export im_module_list for module discovery */
G_MODULE_EXPORT void
im_module_list(const GtkIMContextInfo ***contexts, guint *n_contexts)
{
  static const GtkIMContextInfo im_bridge_info = {
    "im-bridge",         /* id */
    "IM Bridge",         /* name */
    "keyman",             /* domainname */
    "gtk30",             /* domain_dirname */
    "*",                  /* default_locales */
  };

  static const GtkIMContextInfo *info_list[] = {
    &im_bridge_info,
  };

  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS(info_list);
}
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
/* GIO-style module entry points expected by GTK4's module loader. */

G_MODULE_EXPORT void
g_io_module_load(GIOModule *module)
{
  g_type_module_use(G_TYPE_MODULE(module));

  /* If this were a G_DEFINE_DYNAMIC_TYPE we'd call the generated
   * register function here (e.g. gtk_im_bridge_context_register_type
   * (G_TYPE_MODULE (module))). We still ensure the type is known by
   * calling the get_type helper and initialize module state.
   */
  // gtk_im_bridge_context_get_type();

  /* Register the extension point implementation: name, required type,
   * extension name, priority. */
  g_io_extension_point_implement("gtk-im-module",
                                 gtk_im_bridge_context_get_type(),
                                 "im-bridge",
                                 1000);

  im_module_init(G_TYPE_MODULE(module));
}

G_MODULE_EXPORT void
g_io_module_unload(GIOModule *module)
{
  GTypeModule * gtype_module = G_TYPE_MODULE(module);
  LOG_ENTER(__FUNCTION__, "module=%p, use_count=%d", (void *)module, gtype_module->use_count);
  im_module_exit();
  g_type_module_unuse(G_TYPE_MODULE(module));
  LOG_EXIT(__FUNCTION__, "");
}

G_MODULE_EXPORT char **
g_io_module_query(void)
{
  static char *eps[] = { "gtk-im-module", NULL };
  return g_strdupv(eps);
}
#endif
