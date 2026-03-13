#include <gtk/gtk.h>

static void
on_activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *entry;
  gchar title[64];

  window = gtk_application_window_new(app);
  g_snprintf(title, sizeof(title), "Test App for GTK %d",
             gtk_get_major_version());
  gtk_window_set_title(GTK_WINDOW(window), title);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(box), 10);
  gtk_container_add(GTK_CONTAINER(window), box);

  label = gtk_label_new("Enter text:");
  gtk_container_add(GTK_CONTAINER(box), label);

  entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Type something...");
  gtk_container_add(GTK_CONTAINER(box), entry);

  gtk_widget_show_all(window);
}

int
main(int argc, char *argv[])
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.simpleapp", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
