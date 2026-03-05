#include <gtk/gtk.h>
#include "im-bridge.h"

int main(int argc, char **argv)
{
  g_print("GTK-IM-Bridge test started\n");
  
  // Don't call gtk_init() to avoid auto-loading system IM modules
  // which would cause type registration conflicts

  g_print("Creating IM context...\n");
  GtkIMContext *ctx = gtk_im_bridge_context_new();

  if (!ctx) {
    g_printerr("Failed to create context\n");
    return 1;
  }

  g_print("Testing IM context methods...\n");
  gtk_im_context_focus_in(ctx);
  gtk_im_context_set_use_preedit(ctx, TRUE);

  char *pre = NULL;
  PangoAttrList *attrs = NULL;
  int cursor = 0;
  gtk_im_context_get_preedit_string(ctx, &pre, &attrs, &cursor);
  g_print("Preedit string: '%s' (cursor at %d)\n", pre ? pre : "", cursor);
  
  if (attrs)
    pango_attr_list_unref(attrs);
  if (pre)
    g_free(pre);

  gtk_im_context_reset(ctx);
  gtk_im_context_focus_out(ctx);

  g_object_unref(ctx);
  g_print("Test passed!\n");
  return 0;
}
