#include <gtk/gtk.h>
#include <gmodule.h>
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
