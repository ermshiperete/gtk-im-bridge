#include "im-bridge.h"
#include "logging.h"
#include <dlfcn.h>
#include <stdlib.h>

typedef struct _GtkImBridgeContextPrivate GtkImBridgeContextPrivate;


int COUNTER = 0;
struct _GtkImBridgeContextPrivate
{
  int id;
  GtkIMContext *child_context;
  GtkWidget *client_widget;
  gboolean preedit_visible;
  gchar* preedit_text;
  gint preedit_cursor_pos;
  gint signal_commit_id;
  gint signal_delete_surrounding_id;
#if GTK_CHECK_VERSION(4, 22, 0)
  gint signal_invalid_composition_id;
#endif
  gint signal_preedit_changed_id;
  gint signal_preedit_end_id;
  gint signal_preedit_start_id;
  gint signal_retrieve_surrounding_id;
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

static void
gtk_im_bridge_context_init(GtkImBridgeContext *self)
{
  LOG_ENTER("gtk_im_bridge_context_init", "self=%p", (void *)self);

  self->priv = gtk_im_bridge_context_get_instance_private(self);
  self->priv->id = ++COUNTER;

  self->priv->child_context = NULL;
  self->priv->client_widget = NULL;
  self->priv->preedit_visible = FALSE;
  self->priv->preedit_text = NULL;
  self->priv->preedit_cursor_pos = 0;

  const char *env_module = g_getenv("IM_BRIDGE_MODULE");
  const char *context_id = env_module ? env_module : "ibus";
  LOG_MESSAGE("Created GtkImBridgeContext instance with id=%d and child %s", self->priv->id, context_id);

  GIOExtensionPoint *extensionPoint;
  GIOExtension *extension;

  extensionPoint = g_io_extension_point_lookup("gtk-im-module");
  extension = g_io_extension_point_get_extension_by_name(extensionPoint, context_id);
  if (extension)
  {
    GType type = g_io_extension_get_type(extension);
    self->priv->child_context = g_object_new(type, NULL);
  }

  if (self->priv->child_context != NULL)
    {
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

  if (self->priv->client_widget != NULL)
    {
      g_object_unref(self->priv->client_widget);
      self->priv->client_widget = NULL;
    }

  if (self->priv->child_context != NULL)
    {
      g_object_unref(self->priv->child_context);
      self->priv->child_context = NULL;
    }

  LOG_EXIT("gtk_im_bridge_context_finalize", "");

  G_OBJECT_CLASS(gtk_im_bridge_context_parent_class)->finalize(object);
}

/* ==============================================
   Virtual method implementations
   ============================================== */

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

static gboolean
gtk_im_bridge_context_filter_keypress(GtkIMContext *context,
                                       GdkEvent *event)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  LOG_ENTER("filter_keypress", "event=%p type=%d", (void *)event,
            event ? gdk_event_get_event_type(event) : 0);

  gboolean result = FALSE;

  if (self->priv->child_context != NULL && event != NULL)
  {
    result = gtk_im_context_filter_keypress(self->priv->child_context, event);
  }

  LOG_EXIT("filter_keypress", "result=%s", result ? "TRUE" : "FALSE");
  return result;
}

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

/* ==============================================
   Class initialization
   ============================================== */

static void
gtk_im_bridge_context_class_init(GtkImBridgeContextClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(klass);

  gobject_class->finalize = gtk_im_bridge_context_finalize;

  im_context_class->set_client_widget = gtk_im_bridge_context_set_client_widget;
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
  im_context_class->activate_osk_with_event = gtk_im_bridge_context_activate_osk;
}

/* ==============================================
   Public API
   ============================================== */

GtkIMContext *
gtk_im_bridge_context_new(void)
{
  return g_object_new(GTK_IM_BRIDGE_TYPE_CONTEXT, NULL);
}
