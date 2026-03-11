#include <gtk/gtk.h>
#include <gio/gio.h>
#include <stdio.h>

static void
on_activate(GtkApplication *app, gpointer user_data)
{
  /* Just activate to trigger module loading */
  g_application_quit(G_APPLICATION(app));
}

int main(int argc, char *argv[])
{
  GIOExtensionPoint *extension_point;
  GList *extensions;
  GList *iter;
  GtkApplication *app;
  int status;

  /* Initialize GTK by creating and running a minimal application */
  app = gtk_application_new("com.example.list-gtk-im-modules", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  /* Look up the gtk-im-module extension point */
  extension_point = g_io_extension_point_lookup("gtk-im-module");
  if (!extension_point)
    {
      fprintf(stderr, "Error: gtk-im-module extension point not found\n");
      return 1;
    }

  /* Get all registered extensions */
  extensions = g_io_extension_point_get_extensions(extension_point);
  if (!extensions)
    {
      printf("No GTK IM modules registered\n");
      return 0;
    }

  printf("Registered GTK IM modules:\n");
  printf("===========================\n");

  for (iter = extensions; iter != NULL; iter = iter->next)
    {
      GIOExtension *extension = (GIOExtension *)iter->data;
      const char *name = g_io_extension_get_name(extension);
      int priority = g_io_extension_get_priority(extension);
      GType type = g_io_extension_get_type(extension);
      const char *type_name = g_type_name(type);

      printf("  Name: %s\n", name);
      printf("  Priority: %d\n", priority);
      printf("  Type: %s\n", type_name);
      printf("\n");
    }

  return 0;
}
