#include <glib.h>
#include <gmodule.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

/* Minimal GTypeModule implementation for testing */
typedef struct {
  GTypeModule parent;
  GModule *gmodule;
} TestTypeModule;

typedef struct {
  GTypeModuleClass parent_class;
} TestTypeModuleClass;

static GType test_type_module_get_type(void);

G_DEFINE_TYPE(TestTypeModule, test_type_module, G_TYPE_TYPE_MODULE)

static gboolean
test_type_module_load(GTypeModule *module)
{
  printf("test_type_module_load called\n");
  /* Module is already loaded, just return TRUE */
  return TRUE;
}

static void
test_type_module_unload(GTypeModule *module)
{
  printf("test_type_module_unload called\n");
}

static void
test_type_module_init(TestTypeModule *self)
{
  printf("TestTypeModule instance init\n");
}

static void
test_type_module_class_init(TestTypeModuleClass *klass)
{
  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS(klass);

  printf("TestTypeModule class init\n");
  module_class->load = test_type_module_load;
  module_class->unload = test_type_module_unload;
}

int main(int argc, char *argv[])
{
  GModule *module = NULL;
  gpointer init_func = NULL;
  gpointer create_func = NULL;
  GtkIMContext *context = NULL;

  /* Initialize GTK */
  gtk_init(&argc, &argv);

  printf("=== GTK IM Module Dynamic Load Test ===\n");
  printf("GTK version: %d.%d.%d\n",
         gtk_get_major_version(),
         gtk_get_minor_version(),
         gtk_get_micro_version());

  /* Try to load im-ibus module */
  printf("\nAttempting to load im-ibus module...\n");
  module = g_module_open("im-ibus.so", G_MODULE_BIND_LAZY);

  if (!module) {
    /* Try standard GTK3 module path */
    printf("Failed with im-ibus.so, trying /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-ibus.so...\n");
    module = g_module_open("/usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/im-ibus.so", G_MODULE_BIND_LAZY);
  }

  if (!module) {
    printf("ERROR: Failed to load module: %s\n", g_module_error());
    return 1;
  }

  printf("Module loaded successfully: %s\n", g_module_name(module));

  /* Load im_module_init function */
  printf("\nLoading im_module_init...\n");
  if (!g_module_symbol(module, "im_module_init", &init_func)) {
    printf("ERROR loading im_module_init: %s\n", g_module_error());
    g_module_close(module);
    return 1;
  }

  printf("im_module_init loaded at %p\n", init_func);

  /* Load im_module_create function */
  printf("Loading im_module_create...\n");
  if (!g_module_symbol(module, "im_module_create", &create_func)) {
    printf("ERROR loading im_module_create: %s\n", g_module_error());
    g_module_close(module);
    return 1;
  }

  printf("im_module_create loaded at %p\n", create_func);

  /* Call im_module_init */
  printf("\nCalling im_module_init...\n");
  void (*im_module_init)(GTypeModule *module) =
      (void (*)(GTypeModule *module))init_func;

  /* Create a proper GTypeModule */
  GTypeModule *gtype_module = g_object_new(test_type_module_get_type(), NULL);
  if (!gtype_module) {
    printf("ERROR: Failed to create TestTypeModule\n");
    g_module_close(module);
    return 1;
  }

  printf("TestTypeModule created at %p\n", (void *)gtype_module);

  /* Call the init function with the proper module */
  printf("Calling im_module_init with TestTypeModule...\n");
  im_module_init(gtype_module);
  printf("im_module_init completed successfully\n");

  /* Call im_module_create */
  printf("\nCalling im_module_create('ibus')...\n");
  GtkIMContext *(*im_module_create)(const char *context_id) =
      (GtkIMContext * (*)(const char *context_id))create_func;

  context = im_module_create("ibus");

  if (context) {
    printf("SUCCESS: im_module_create returned context at %p\n",
           (void *)context);
    printf("Context type: %s\n", G_OBJECT_TYPE_NAME(context));
    g_object_unref(context);
  } else {
    printf("WARNING: im_module_create returned NULL\n");
  }

  /* Cleanup */
  /* Note: Don't unref gtype_module - GTypeModule has special lifecycle management.
     Once types are registered with it, it shouldn't be disposed. */
  g_module_close(module);

  printf("\n=== Test completed ===\n");
  return 0;
}
