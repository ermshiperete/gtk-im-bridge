#include "im-bridge.h"
#include "logging.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <glib/gstdio.h>

#if GTK_CHECK_VERSION(4, 0, 0)
#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#endif

#ifdef GDK_WINDOWING_BROADWAY
#include <gdk/broadway/gdkbroadway.h>
#endif

#ifdef GDK_WINDOWING_WIN32
#include <gdk/win32/gdkwin32.h>
#endif
#else // GTK_CHECK_VERSION(4,0,0)
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#ifdef GDK_WINDOWING_BROADWAY
#include <gdk/gdkbroadway.h>
#endif

#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
#endif
#include <locale.h>
#endif

typedef struct _GtkImBridgeContextPrivate GtkImBridgeContextPrivate;


int COUNTER = 0;
struct _GtkImBridgeContextPrivate
{
  int id;
  GtkIMContext *child_context;

  gint signal_commit_id;
  gint signal_delete_surrounding_id;
  gint signal_preedit_changed_id;
  gint signal_preedit_end_id;
  gint signal_preedit_start_id;
  gint signal_retrieve_surrounding_id;
#if GTK_CHECK_VERSION(4, 22, 0)
  gint signal_invalid_composition_id;
#endif
#if !GTK_CHECK_VERSION(4,0,0)
  GModule *im_module;
#endif
};

struct _GtkImBridgeContext
{
  GtkIMContext parent_instance;
  GtkImBridgeContextPrivate *priv;
};

G_DEFINE_FINAL_TYPE_WITH_PRIVATE(GtkImBridgeContext,
                                   gtk_im_bridge_context,
                                   GTK_TYPE_IM_CONTEXT)

/* ==============================================
   Callback handlers for child signal forwarding
   ============================================== */

static void
on_commit_from_child(GtkIMContext *child,
               gchar *str,
               gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_ENTER(__FUNCTION__, "");

  LOG_SIGNAL("child::commit", "text='%s'", str);
  g_signal_emit(self, self->priv->signal_commit_id, 0, str);
  LOG_EXIT(__FUNCTION__, "");
}

static gboolean
on_delete_surrounding_from_child(GtkIMContext *child_ctx,
                                 gint offset,
                                 gint nchars,
                                 gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  gboolean result;

  LOG_SIGNAL("child::delete-surrounding", "offset=%d nchars=%u", offset, nchars);
  g_signal_emit(self, self->priv->signal_delete_surrounding_id, 0, offset, nchars, &result);
  LOG_EXIT(__FUNCTION__, "%s", result ? "TRUE" : "FALSE");
  return result;
}

#if GTK_CHECK_VERSION(4, 22, 0)
static gboolean
on_invalid_composition_from_child(GtkIMContext *child_ctx, gchar *str, gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  gboolean result;

  LOG_SIGNAL("child::invalid-composition", "str=%s", str);
  g_signal_emit(self, self->priv->signal_invalid_composition_id, 0, str, &result);
  LOG_EXIT(__FUNCTION__, "%s", result ? "TRUE" : "FALSE");
  return result;
}
#endif

static void on_preedit_changed_from_child(GtkIMContext *child_ctx,
                                          gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_SIGNAL("child::preedit-changed", "");
  g_signal_emit(self, self->priv->signal_preedit_changed_id, 0);
  LOG_EXIT(__FUNCTION__, "");
}

static void on_preedit_end_from_child(GtkIMContext *child_ctx,
                                    gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_SIGNAL("child::preedit-end", "");
  g_signal_emit(self, self->priv->signal_preedit_end_id, 0);
  LOG_EXIT(__FUNCTION__, "");
}

static void on_preedit_start_from_child(GtkIMContext *child_ctx,
                                    gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_SIGNAL("child::preedit-start", "");
  g_signal_emit(self, self->priv->signal_preedit_start_id, 0);
  LOG_EXIT(__FUNCTION__, "");
}

static gboolean
on_retrieve_surrounding_from_child(GtkIMContext *child_ctx,
                                           gpointer user_data)
{
  LOG_ENTER(__FUNCTION__, "");
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  gboolean result;
  LOG_SIGNAL("child::retrieve-surrounding", "");
  g_signal_emit(self, self->priv->signal_retrieve_surrounding_id, 0, &result);
  LOG_EXIT(__FUNCTION__, "%s", result ? "TRUE" : "FALSE");
  return result;
}

/* ==============================================
   GObject lifecycle methods
   ============================================== */

// basically a copy of `match_backend` in https://github.com/GNOME/gtk/blob/1cabb42dc61ddfe11302efbf9889d664f97a0b2e/gtk/gtkimmodule.c
static gboolean
_match_backend(const char *context_id)
{
#if GTK_CHECK_VERSION(4, 0, 0)
  if (g_strcmp0(context_id, "wayland") == 0)
  {
#ifdef GDK_WINDOWING_WAYLAND
    GdkDisplay *display = gdk_display_get_default();
    return GDK_IS_WAYLAND_DISPLAY(display) &&
           gdk_wayland_display_query_registry(display,
                                              "zwp_text_input_manager_v3");
#else
    return FALSE;
#endif
  }

  if (g_strcmp0(context_id, "broadway") == 0)
#ifdef GDK_WINDOWING_BROADWAY
    return GDK_IS_BROADWAY_DISPLAY(gdk_display_get_default());
#else
    return FALSE;
#endif

  if (g_strcmp0(context_id, "ime") == 0)
#ifdef GDK_WINDOWING_WIN32
    return GDK_IS_WIN32_DISPLAY(gdk_display_get_default());
#else
    return FALSE;
#endif

  if (g_strcmp0(context_id, "quartz") == 0)
#ifdef GDK_WINDOWING_MACOS
    return GDK_IS_MACOS_DISPLAY(gdk_display_get_default());
#else
    return FALSE;
#endif

  if (g_strcmp0(context_id, "android") == 0)
#ifdef GDK_WINDOWING_ANDROID
    return GDK_IS_ANDROID_DISPLAY(gdk_display_get_default());
#else
    return FALSE;
#endif

#else // GTK_CHECK_VERSION(4, 0, 0)
#ifdef GDK_WINDOWING_WAYLAND
  if (g_strcmp0(context_id, "wayland") == 0)
  {
    GdkDisplay *display = gdk_display_get_default();

    return GDK_IS_WAYLAND_DISPLAY(display) &&
           gdk_wayland_display_query_registry(display,
                                              "zwp_text_input_manager_v3");
  }
  if (g_strcmp0(context_id, "waylandgtk") == 0)
  {
    GdkDisplay *display = gdk_display_get_default();

    return GDK_IS_WAYLAND_DISPLAY(display) &&
           gdk_wayland_display_query_registry(display,
                                              "gtk_text_input_manager");
  }
#endif

#ifdef GDK_WINDOWING_BROADWAY
  if (g_strcmp0(context_id, "broadway") == 0)
    return GDK_IS_BROADWAY_DISPLAY(gdk_display_get_default());
#endif

#ifdef GDK_WINDOWING_X11
  if (g_strcmp0(context_id, "xim") == 0)
    return GDK_IS_X11_DISPLAY(gdk_display_get_default());
#endif

#ifdef GDK_WINDOWING_WIN32
  if (g_strcmp0(context_id, "ime") == 0)
    return GDK_IS_WIN32_DISPLAY(gdk_display_get_default());
#endif
#endif // GTK_CHECK_VERSION(4, 0, 0)

  return TRUE;
}

#if ! GTK_CHECK_VERSION(4, 0, 0)
// Load all im-*.so files from the immodules directory
static GSList* _load_modules() {
  const char *immodules_dir = "/usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/immodules/";
  GDir *dir;
  GError *error = NULL;
  GSList *infos = NULL;

  dir = g_dir_open(immodules_dir, 0, &error);
  if (!dir) {
    /* In case the directory doesn't exist, we allow returning NULL.
     */
    if (error) {
      g_error_free(error);
    }
    return NULL;
  }

  const char *filename;
  while ((filename = g_dir_read_name(dir)) != NULL) {
    /* Look for files matching im-*.so pattern */
    if (g_str_has_prefix(filename, "im-") && g_str_has_suffix(filename, ".so")) {
      gchar *full_path = g_build_filename(immodules_dir, filename, NULL);
      g_message("Found IM module: %s", full_path);
      /* Store the full path in infos for later loading */
      infos = g_slist_prepend(infos, full_path);
    }
  }

  g_dir_close(dir);
  return infos;
}

/* Match @locale against @against.
 *
 * 'en_US' against "en_US"       => 4
 * 'en_US' against "en"          => 3
 * 'en', "en_UK" against "en_US" => 2
 *  all locales, against "*"     => 1
 */
static gint
_match_locale(const gchar *locale,
             const gchar *against,
             size_t against_len)
{
  LOG_MESSAGE("%s: Checking %s against %s (%d)", __FUNCTION__, locale, against, against_len);
  if (strcmp(against, "*") == 0)
    return 1;

  if (g_ascii_strcasecmp(locale, against) == 0)
    return 4;

  if (g_ascii_strncasecmp(locale, against, 2) == 0)
    return (against_len == 2) ? 3 : 2;

  return 0;
}

#define SIMPLE_ID "gtk-im-context-simple"
#define NONE_ID "gtk-im-context-none"

static gchar *
_get_immodule(GtkImBridgeContext * self, gchar * *context_ids, GSList *modules_list)
{
  LOG_ENTER(__FUNCTION__, "");
  gchar *context_id = NULL;
  while (context_ids && *context_ids) {
    if (g_strcmp0(*context_ids, SIMPLE_ID) == 0)
      return SIMPLE_ID;
    else if (g_strcmp0(*context_ids, NONE_ID) == 0)
      return NONE_ID;
    else if (g_strcmp0(*context_ids, "im-bridge") != 0) {
      /* Iterate over modules_list and find *context_ids */
      for (GSList *l = modules_list; l != NULL; l = l->next) {
        gchar *module_path = (gchar *)l->data;
        GModule *gmodule = g_module_open(module_path, G_MODULE_BIND_LAZY);

        if (gmodule) {
          gpointer list_func = NULL;
          if (g_module_symbol(gmodule, "im_module_list", &list_func)) {
            void (*im_module_list)(const GtkIMContextInfo ***, guint *) = (void (*)(const GtkIMContextInfo ***, guint *))list_func;
            const GtkIMContextInfo **contexts;
            guint n_contexts;
            im_module_list(&contexts, &n_contexts);

            for (guint i = 0; i < n_contexts; i++) {
              if (g_strcmp0(contexts[i]->context_id, *context_ids) == 0) {
                context_id = g_strdup(*context_ids);
                self->priv->im_module = gmodule;
                LOG_EXIT(__FUNCTION__, "found var; context_id=%s", context_id);
                return context_id;
              }
            }
          }
          g_module_close(gmodule);
        }
      }
    }
    context_ids++;
  }

  // find the first module that matches backend and locale
  gchar *tmp_locale, *tmp;
  tmp_locale = g_strdup(setlocale(LC_CTYPE, NULL));

  /* Strip the locale code down to the essentials
   */
  tmp = strchr(tmp_locale, '.');
  if (tmp)
    *tmp = '\0';
  tmp = strchr(tmp_locale, '@');
  if (tmp)
    *tmp = '\0';

  GSList *tmp_list = modules_list;
  gint best_goodness = 0;
  while (tmp_list) {
    gchar *module_path = (gchar *)tmp_list->data;
    LOG_MESSAGE("_get_immodule: checking %s", module_path);
    GModule *gmodule = g_module_open(module_path, G_MODULE_BIND_LAZY);

    if (gmodule) {
      gboolean found = FALSE;
      gpointer list_func = NULL;
      if (g_module_symbol(gmodule, "im_module_list", &list_func)) {
        void (*im_module_list)(const GtkIMContextInfo ***, guint *) = (void (*)(const GtkIMContextInfo ***, guint *))list_func;
        const GtkIMContextInfo **contexts;
        guint n_contexts;
        im_module_list(&contexts, &n_contexts);

        for (guint i = 0; i < n_contexts; i++) {
          LOG_MESSAGE("_get_immodule: processing %s", contexts[i]->context_id);
          if (g_strcmp0(contexts[i]->context_id, "im-bridge") == 0) {
            LOG_MESSAGE("%s: skipping im-bridge", __FUNCTION__);
            continue;
          }
          if (!_match_backend(contexts[i]->context_id)) {
            LOG_MESSAGE("%s: backend doesn't match", __FUNCTION__);
            continue;
          }

          const gchar *p;
          p = contexts[i]->default_locales;
          LOG_MESSAGE("%s: default_locales: %s, locale: %s", __FUNCTION__, p, tmp_locale);
          while (p) {
            const gchar *q = strchr(p, ':');
            gint goodness = _match_locale(tmp_locale, p, q ? (size_t)(q - p) : strlen(p));

            if (goodness > best_goodness) {
              LOG_MESSAGE("_get__immodule: goodness %d > best_goodness %d", goodness, best_goodness);
              if (context_id) {
                g_free(context_id);
              }
              context_id = g_strdup(contexts[i]->context_id);
              best_goodness = goodness;
              if (self->priv->im_module) {
                g_module_close(self->priv->im_module);
              }
              self->priv->im_module = gmodule;
              found = TRUE;
            }

            p = q ? q + 1 : NULL;
          }
        }
      }
      if (!found) {
        g_module_close(gmodule);
      }
    }
    tmp_list = tmp_list->next;
  }

  g_free(tmp_locale);

  LOG_EXIT(__FUNCTION__, "default; context_id=%s", context_id);
  return context_id;
}

static gchar *
_get_context_id(GtkImBridgeContext *self) {
  gchar **immodules;
  GSList *modules_list;

  const gchar *envvar = g_getenv("IM_BRIDGE_MODULE");
  gchar how[100];
  g_strlcpy(how, "env variable", 100);
  modules_list = _load_modules();

  if (envvar) {
    immodules = g_strsplit(envvar, ":", 0);
  } else {
    immodules = NULL;
  }
  char *context_id = _get_immodule(self, immodules, modules_list);
  g_strfreev(immodules);
  g_slist_free(modules_list);
  return context_id;
}
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
static void
_create_context_instance(GtkImBridgeContext *self)
{
  LOG_ENTER("_create_context_instance", "self=%p", (void *)self);

  self->priv = gtk_im_bridge_context_get_instance_private(self);
  self->priv->id = ++COUNTER;

  self->priv->child_context = NULL;

  const char *context_id = g_getenv("IM_BRIDGE_MODULE");
  GIOExtensionPoint *extensionPoint;
  GIOExtension *extension = NULL;
  gchar how[100];
  g_strlcpy(how, "env variable", 100);

  extensionPoint = g_io_extension_point_lookup("gtk-im-module");

  if (!context_id) {
    /* Find the extension with highest priority (excluding im-bridge) */
    GList *extensions = g_io_extension_point_get_extensions(extensionPoint);
    int best_priority = G_MININT;
    const char *best_context_id = NULL;

    for (GList *l = extensions; l != NULL; l = l->next) {
      GIOExtension *ext = (GIOExtension *)l->data;
      const char *ext_name = g_io_extension_get_name(ext);
      int ext_priority = g_io_extension_get_priority(ext);

      /* Skip im-bridge itself */
      if (g_strcmp0(ext_name, "im-bridge") == 0)
        continue;

      if (ext_priority > best_priority && _match_backend(ext_name)) {
        best_priority = ext_priority;
        best_context_id = ext_name;
      }
    }

    if (best_context_id == NULL) {
      /* Fallback to ibus if no other suitable extension found */
      context_id = "ibus";
      g_strlcpy(how, "fallback", 100);
    } else {
      context_id = best_context_id;
      g_snprintf(how, 100, "highest priority: %d", best_priority);
    }
  }

  extension = g_io_extension_point_get_extension_by_name(extensionPoint, context_id);
  LOG_MESSAGE("Created GtkImBridgeContext instance with id=%d and child %s (%s)", self->priv->id, context_id, how);

  if (extension) {
    GType type = g_io_extension_get_type(extension);
    self->priv->child_context = g_object_new(type, NULL);
  }
  LOG_EXIT("_create_context_instance", "");
}
#else
/* GTK3 version */
static void
_create_context_instance(GtkImBridgeContext *self)
{
  LOG_ENTER("_create_context_instance", "self=%p", (void *)self);

  self->priv = gtk_im_bridge_context_get_instance_private(self);
  self->priv->id = ++COUNTER;

  self->priv->child_context = NULL;

  char *context_id = _get_context_id(self);
  if (context_id) {
    gpointer init_func = NULL;
    gpointer create_func = NULL;
    if (!g_module_symbol(self->priv->im_module, "im_module_init", &init_func) ||
        !g_module_symbol(self->priv->im_module, "im_module_create", &create_func)) {
      LOG_ERROR("Error loading funcs: %s", g_module_error());
      return;
    }
    void (*im_module_init)(GTypeModule *module) = (void (*)(GTypeModule *module))init_func;
    im_module_init(G_TYPE_MODULE(self->priv->im_module));
    GtkIMContext *(*im_module_create)(const char *context_id) = (GtkIMContext * (*)(const char *context_id)) create_func;
    self->priv->child_context = im_module_create(context_id);
    g_free(context_id);
  }
  else
  {
    LOG_ERROR("Can't find suitable context_id.");
    return;
  }
  LOG_EXIT("_create_context_instance", "Created GtkImBridgeContext instance with id=%d and child %s", self->priv->id, context_id);
}
#endif

static void
gtk_im_bridge_context_init(GtkImBridgeContext *self)
{
  LOG_ENTER("gtk_im_bridge_context_init", "");
  _create_context_instance(self);

  if (self->priv->child_context != NULL) {
    /* Connect to child signals */
    g_signal_connect(self->priv->child_context, "commit",
      G_CALLBACK(on_commit_from_child), self);
    g_signal_connect(self->priv->child_context, "delete-surrounding",
      G_CALLBACK(on_delete_surrounding_from_child), self);
    g_signal_connect(self->priv->child_context, "preedit-changed",
      G_CALLBACK(on_preedit_changed_from_child), self);
    g_signal_connect(self->priv->child_context, "preedit-end",
      G_CALLBACK(on_preedit_end_from_child), self);
    g_signal_connect(self->priv->child_context, "preedit-start",
      G_CALLBACK(on_preedit_start_from_child), self);
    g_signal_connect(self->priv->child_context, "retrieve-surrounding",
      G_CALLBACK(on_retrieve_surrounding_from_child), self);
    self->priv->signal_commit_id =
        g_signal_lookup("commit", GTK_IM_BRIDGE_TYPE_CONTEXT);
    self->priv->signal_delete_surrounding_id =
      g_signal_lookup("delete-surrounding", GTK_IM_BRIDGE_TYPE_CONTEXT);
    self->priv->signal_preedit_changed_id =
      g_signal_lookup("preedit-changed", GTK_IM_BRIDGE_TYPE_CONTEXT);
    self->priv->signal_preedit_end_id =
      g_signal_lookup("preedit-end", GTK_IM_BRIDGE_TYPE_CONTEXT);
    self->priv->signal_preedit_start_id =
      g_signal_lookup("preedit-start", GTK_IM_BRIDGE_TYPE_CONTEXT);
    self->priv->signal_retrieve_surrounding_id =
      g_signal_lookup("retrieve-surrounding", GTK_IM_BRIDGE_TYPE_CONTEXT);
#if GTK_CHECK_VERSION(4, 22, 0)
    g_signal_connect(self->priv->child_context, "invalid-composition",
      G_CALLBACK(on_invalid_composition_from_child), self);
    self->priv->signal_invalid_composition_id =
      g_signal_lookup("invalid-composition", GTK_IM_BRIDGE_TYPE_CONTEXT);
#endif
    LOG_MESSAGE("gtk_im_bridge_context_init: child_context=%p, signal_commit_id=%d, signal_delete_surrounding_id=%d, signal_preeddit_changed_id=%d, signal_preedit_end_id=%d, signal_preedist_start=%d, signal_retrieve_surrounding_id=%d",
                (void *)self->priv->child_context, self->priv->signal_commit_id,
                self->priv->signal_delete_surrounding_id, self->priv->signal_preedit_changed_id,
                self->priv->signal_preedit_end_id, self->priv->signal_preedit_start_id,
                self->priv->signal_retrieve_surrounding_id);
  }

  LOG_EXIT("gtk_im_bridge_context_init", "");
}

static void
gtk_im_bridge_context_finalize(GObject *object)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(object);

  LOG_ENTER("gtk_im_bridge_context_finalize", "self=%p", (void *)self);

  if (self->priv->child_context != NULL) {
    g_object_unref(self->priv->child_context);
    self->priv->child_context = NULL;
  }

  LOG_EXIT("gtk_im_bridge_context_finalize", "");

  G_OBJECT_CLASS(gtk_im_bridge_context_parent_class)->finalize(object);
}

/* ==============================================
   Virtual method implementations
   ============================================== */

#if GTK_CHECK_VERSION(4, 0, 0)
static void
gtk_im_bridge_context_set_client_widget(GtkIMContext *context,
                                        GtkWidget *widget)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("set_client_widget", "widget=%p", (void *)widget);

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_client_widget(self->priv->child_context, widget);
  }

  LOG_EXIT("set_client_widget", "");
}
#else
/* GTK3 version using set_client_window */
static void
gtk_im_bridge_context_set_client_window(GtkIMContext *context,
                                        GdkWindow *window)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("set_client_window", "window=%p", (void *)window);

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_client_window(self->priv->child_context, window);
  }

  LOG_EXIT("set_client_window", "");
}
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
static gboolean
gtk_im_bridge_context_filter_keypress(GtkIMContext *context,
                                       GdkEvent *event)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  LOG_ENTER("filter_keypress", "event=%p type=%d", (void *)event,
            event ? gdk_event_get_event_type(event) : 0);

  gboolean result = FALSE;

  if (self->priv->child_context != NULL && event != NULL) {
    result = gtk_im_context_filter_keypress(self->priv->child_context, event);
  }

  LOG_EXIT("filter_keypress", "result=%s", result ? "TRUE" : "FALSE");
  return result;
}
#else
/* GTK3 version with GdkEventKey */
static gboolean
gtk_im_bridge_context_filter_keypress(GtkIMContext *context,
                                       GdkEventKey *event)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  LOG_ENTER("filter_keypress", "event=%p type=%d", (void *)event,
            event ? event->type : 0);

  gboolean result = FALSE;

  if (self->priv->child_context != NULL && event != NULL) {
    result = gtk_im_context_filter_keypress(self->priv->child_context, event);
  }

  LOG_EXIT("filter_keypress", "result=%s", result ? "TRUE" : "FALSE");
  return result;
}
#endif

static void
gtk_im_bridge_context_focus_in(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("focus_in", "");

  if (self->priv->child_context != NULL) {
    gtk_im_context_focus_in(self->priv->child_context);
  }

  LOG_EXIT("focus_in", "");
}

static void
gtk_im_bridge_context_focus_out(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("focus_out", "");

  if (self->priv->child_context != NULL) {
    gtk_im_context_focus_out(self->priv->child_context);
  }

  LOG_EXIT("focus_out", "");
}

static void
gtk_im_bridge_context_reset(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("reset", "");

  if (self->priv->child_context != NULL) {
    gtk_im_context_reset(self->priv->child_context);
  }

  LOG_EXIT("reset", "");
}

static void
gtk_im_bridge_context_set_cursor_location(GtkIMContext *context,
                                          GdkRectangle *area)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("set_cursor_location", "area={x=%d y=%d w=%d h=%d}",
            area->x, area->y, area->width, area->height);

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_cursor_location(self->priv->child_context, area);
  }

  LOG_EXIT("set_cursor_location", "");
}

static void
gtk_im_bridge_context_set_use_preedit(GtkIMContext *context,
                                      gboolean use_preedit)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("set_use_preedit", "use_preedit=%s", use_preedit ? "TRUE" : "FALSE");

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_use_preedit(self->priv->child_context, use_preedit);
  }

  LOG_EXIT("set_use_preedit", "");
}

static void
gtk_im_bridge_context_get_preedit_string(GtkIMContext *context,
                                         char **str,
                                         PangoAttrList **attrs,
                                         int *cursor_pos)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("get_preedit_string", "");

  if (self->priv->child_context != NULL) {
    gtk_im_context_get_preedit_string(self->priv->child_context, str, attrs, cursor_pos);
  }

  if (cursor_pos) {
    LOG_EXIT("get_preedit_string", "str='%s' cursor_pos=%d", *str, *cursor_pos);
  } else {
    LOG_EXIT("get_preedit_string", "str='%s', cursor_pos=NULL", *str);
  }
}

#if GTK_CHECK_VERSION(4, 1, 2)
/* GTK4.1.2+ has set_surrounding_with_selection */
static void
gtk_im_bridge_context_set_surrounding_with_selection(GtkIMContext *context,
                                                     const char *text,
                                                     int len,
                                                     int cursor_index,
                                                     int anchor_index)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  char text_repr[64] = {0};

  if (text && len > 0)
    g_snprintf(text_repr, sizeof(text_repr), "%.32s%s", text, len > 32 ? "..." : "");

  LOG_ENTER("set_surrounding_with_selection",
            "text='%s' len=%d cursor=%d anchor=%d",
            text_repr, len, cursor_index, anchor_index);

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_surrounding_with_selection(self->priv->child_context, text, len, cursor_index, anchor_index);
  }

  LOG_EXIT("set_surrounding_with_selection", "");
}

static gboolean
gtk_im_bridge_context_get_surrounding_with_selection(GtkIMContext *context,
                                                     char **text,
                                                     int *cursor_index,
                                                     int *anchor_index)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean result = FALSE;

  LOG_ENTER("get_surrounding_with_selection", "");

  if (self->priv->child_context != NULL) {
    result = gtk_im_context_get_surrounding_with_selection(self->priv->child_context, text, cursor_index, anchor_index);
  }

  LOG_EXIT("get_surrounding_with_selection",
           "result=%s text='%.32s' cursor=%d anchor=%d",
           result ? "TRUE" : "FALSE", *text, *cursor_index, *anchor_index);
  return result;
}
#else
static void
gtk_im_bridge_context_set_surrounding(GtkIMContext *context,
                                      const char *text,
                                      int len,
                                      int cursor_index)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  char text_repr[64] = {0};

  if (text && len > 0)
    g_snprintf(text_repr, sizeof(text_repr), "%.32s%s", text, len > 32 ? "..." : "");

  LOG_ENTER("set_surrounding", "text='%s' len=%d cursor_index=%d",
            text_repr, len, cursor_index);

  if (self->priv->child_context != NULL) {
    gtk_im_context_set_surrounding(self->priv->child_context, text, len, cursor_index);
  }

  LOG_EXIT("set_surrounding", "");
}

static gboolean
gtk_im_bridge_context_get_surrounding(GtkIMContext *context,
                                      char **text,
                                      int *cursor_index)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean result = FALSE;

  LOG_ENTER("get_surrounding", "");

  if (self->priv->child_context != NULL) {
    result = gtk_im_context_get_surrounding(self->priv->child_context, text, cursor_index);
  }

  LOG_EXIT("get_surrounding", "result=%s text='%.32s' cursor_index=%d",
           result ? "TRUE" : "FALSE", *text, *cursor_index);
  return result;
}
#endif

static gboolean
gtk_im_bridge_context_delete_surrounding(GtkIMContext *context,
                                         int offset,
                                         int n_chars)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean success = FALSE;

  LOG_ENTER("delete_surrounding", "offset=%d n_chars=%d", offset, n_chars);

  if (self->priv->child_context != NULL) {
    success = gtk_im_context_delete_surrounding(self->priv->child_context, offset, n_chars);
  }

  LOG_EXIT("delete_surrounding", "success=%s", success ? "TRUE" : "FALSE");
  return success;
}

#if GTK_CHECK_VERSION(4, 0, 0)
static gboolean
gtk_im_bridge_context_activate_osk(GtkIMContext *context,
                                               GdkEvent *event)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean result = FALSE;
  LOG_ENTER("activate_osk", "event=%p type=%d",
            (void *)event,
            event ? gdk_event_get_event_type(event) : 0);
  if (self->priv->child_context != NULL) {
    result = gtk_im_context_activate_osk(self->priv->child_context, event);
  }
  LOG_EXIT("activate_osk", "success=%s", result ? "TRUE" : "FALSE");
  return result;
}
#endif

/* ==============================================
   Class initialization
   ============================================== */

static void
gtk_im_bridge_context_class_init(GtkImBridgeContextClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(klass);

  gobject_class->finalize = gtk_im_bridge_context_finalize;

#if GTK_CHECK_VERSION(4, 0, 0)
  im_context_class->set_client_widget = gtk_im_bridge_context_set_client_widget;
#else
  im_context_class->set_client_window = gtk_im_bridge_context_set_client_window;
#endif
  im_context_class->filter_keypress = gtk_im_bridge_context_filter_keypress;
  im_context_class->focus_in = gtk_im_bridge_context_focus_in;
  im_context_class->focus_out = gtk_im_bridge_context_focus_out;
  im_context_class->reset = gtk_im_bridge_context_reset;
  im_context_class->set_cursor_location = gtk_im_bridge_context_set_cursor_location;
  im_context_class->set_use_preedit = gtk_im_bridge_context_set_use_preedit;
  im_context_class->get_preedit_string = gtk_im_bridge_context_get_preedit_string;
#if GTK_CHECK_VERSION(4, 1, 2)
  im_context_class->set_surrounding_with_selection = gtk_im_bridge_context_set_surrounding_with_selection;
  im_context_class->get_surrounding_with_selection = gtk_im_bridge_context_get_surrounding_with_selection;
#else
  im_context_class->set_surrounding = gtk_im_bridge_context_set_surrounding;
  im_context_class->get_surrounding = gtk_im_bridge_context_get_surrounding;
#endif
  im_context_class->delete_surrounding = gtk_im_bridge_context_delete_surrounding;
#if GTK_CHECK_VERSION(4, 0, 0)
  im_context_class->activate_osk_with_event = gtk_im_bridge_context_activate_osk;
#endif
}

/* ==============================================
   Public API
   ============================================== */

GtkIMContext *
gtk_im_bridge_context_new(void)
{
  return g_object_new(GTK_IM_BRIDGE_TYPE_CONTEXT, NULL);
}
