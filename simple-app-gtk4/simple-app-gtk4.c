#include <gtk/gtk.h>

static void
on_activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *entry;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Simple Text Entry");
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_top(box, 10);
  gtk_widget_set_margin_bottom(box, 10);
  gtk_widget_set_margin_start(box, 10);
  gtk_widget_set_margin_end(box, 10);
  gtk_window_set_child(GTK_WINDOW(window), box);

  label = gtk_label_new("Enter text:");
  gtk_box_append(GTK_BOX(box), label);

  entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Type something...");
  gtk_box_append(GTK_BOX(box), entry);

  gtk_window_present(GTK_WINDOW(window));
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
