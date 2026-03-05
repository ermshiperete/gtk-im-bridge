#include "im-bridge.h"
#include "logging.h"
#include "ibus-setup.h"
#include <ibus.h>

typedef struct _GtkImBridgeContextPrivate GtkImBridgeContextPrivate;

int COUNTER = 0;
struct _GtkImBridgeContextPrivate
{
  int id;
  IBusInputContext *ibus_context;
  GtkWidget *client_widget;
  gboolean preedit_visible;
  gchar* preedit_text;
  gint preedit_cursor_pos;
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
   Callback handlers for IBus signal forwarding
   ============================================== */

static void
on_ibus_commit_text(IBusInputContext *ibus_ctx,
                    IBusText *text,
                    gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  const char *str = ibus_text_get_text(text);

  LOG_SIGNAL("ibus::commit-text", "text='%s'", str);
  g_signal_emit_by_name(self, "commit", str);
}

static void
on_ibus_show_preedit_text(IBusInputContext *ibus_ctx,
                      gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_SIGNAL("ibus::show-preedit-text", "");
  self->priv->preedit_visible = TRUE;
  g_signal_emit_by_name(self, "show-preedit-text");
}

static void
on_ibus_update_preedit_text(IBusInputContext *ibus_ctx,
                            IBusText *text,
                            guint cursor_pos,
                            gboolean visible,
                            gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  self->priv->preedit_text = g_strdup(ibus_text_get_text(text));
  self->priv->preedit_cursor_pos = cursor_pos;
  LOG_SIGNAL("ibus::update-preedit-text", "text=%s cursor_pos=%u visible=%s",
             self->priv->preedit_text, self->priv->preedit_cursor_pos, visible ? "TRUE" : "FALSE");
  g_signal_emit_by_name(self, "update-preedit-text");
}

static void
on_ibus_hide_preedit_text(IBusInputContext *ibus_ctx,
                    gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  LOG_SIGNAL("ibus::hide-preedit-text", "");
  self->priv->preedit_visible = FALSE;
  g_signal_emit_by_name(self, "hide-preedit-text");
}

static gboolean
on_ibus_delete_surrounding_text(IBusInputContext *ibus_ctx,
                           gint offset,
                           guint nchars,
                           gpointer user_data)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(user_data);
  gboolean result;

  LOG_SIGNAL("ibus::delete-surrounding-text", "offset=%d nchars=%u", offset, nchars);
  g_signal_emit_by_name(self, "delete-surrounding-text", offset, nchars, &result);
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
  self->priv->ibus_context = ibus_setup_get_context();
  if (self->priv->ibus_context != NULL)
    {
      /* Take our own reference so finalize can unref safely without
       * invalidating the static context held by ibus-setup. */
      g_object_ref(self->priv->ibus_context);
    }
  self->priv->client_widget = NULL;
  self->priv->preedit_visible = FALSE;
  self->priv->preedit_text = NULL;
  self->priv->preedit_cursor_pos = 0;

  LOG_MESSAGE("Created GtkImBridgeContext instance with id=%d", self->priv->id);

  if (self->priv->ibus_context != NULL)
    {
      /* Connect to IBus signals */
      g_signal_connect(self->priv->ibus_context, "commit-text",
                       G_CALLBACK(on_ibus_commit_text), self);
      g_signal_connect(self->priv->ibus_context, "show-preedit-text",
                       G_CALLBACK(on_ibus_show_preedit_text), self);
      g_signal_connect(self->priv->ibus_context, "update-preedit-text",
                       G_CALLBACK(on_ibus_update_preedit_text), self);
      g_signal_connect(self->priv->ibus_context, "hide-preedit-text",
                       G_CALLBACK(on_ibus_hide_preedit_text), self);
      g_signal_connect(self->priv->ibus_context, "delete-surrounding-text",
                       G_CALLBACK(on_ibus_delete_surrounding_text), self);
      LOG_SIGNAL("gtk-context-created", "ibus_context=%p", (void *)self->priv->ibus_context);
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

  if (self->priv->ibus_context != NULL)
    {
      g_object_unref(self->priv->ibus_context);
      self->priv->ibus_context = NULL;
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

  if (self->priv->client_widget != NULL)
    {
      g_object_unref(self->priv->client_widget);
    }

  self->priv->client_widget = widget ? g_object_ref(widget) : NULL;

  /* IBus doesn't have set_client_widget, but we log and cache it */

  LOG_EXIT("set_client_widget", "");
}

static gboolean
gtk_im_bridge_context_filter_keypress(GtkIMContext *context,
                                       GdkEvent *event)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean result = FALSE;
  guint keyval = GDK_KEY_VoidSymbol;
  GdkModifierType state = 0;

  LOG_ENTER("filter_keypress", "event=%p type=%d", (void *)event,
            event ? gdk_event_get_event_type(event) : 0);

  if (self->priv->ibus_context != NULL && event != NULL)
    {
      keyval = gdk_key_event_get_keyval(event);
      state = gdk_event_get_modifier_state(event);

      result = ibus_input_context_process_key_event(
        self->priv->ibus_context,
        keyval,
        0, /* keycode */
        state);

      LOG_SIGNAL("keypress-process", "keyval=0x%x state=0x%x result=%s",
                 keyval, state, result ? "TRUE" : "FALSE");
    }

  LOG_EXIT("filter_keypress", "result=%s", result ? "TRUE" : "FALSE");
  return result;
}

static void
gtk_im_bridge_context_focus_in(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("focus_in", "");

  if (self->priv->ibus_context != NULL)
    {
      ibus_input_context_focus_in(self->priv->ibus_context);
    }

  LOG_EXIT("focus_in", "");
}

static void
gtk_im_bridge_context_focus_out(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("focus_out", "");

  if (self->priv->ibus_context != NULL)
    {
      ibus_input_context_focus_out(self->priv->ibus_context);
    }

  LOG_EXIT("focus_out", "");
}

static void
gtk_im_bridge_context_reset(GtkIMContext *context)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("reset", "");

  if (self->priv->ibus_context != NULL)
    {
      ibus_input_context_reset(self->priv->ibus_context);
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

  if (self->priv->client_widget == NULL)
  {
    LOG_EXIT("set_cursor_location", "no client widget set, ignoring");
    return;
  }

  // GtkWidget *root;
  // GtkNative *native;
  // graphene_point_t p;
  // int tx = 0, ty = 0;
  // double nx = 0., ny = 0.;
  // root = GTK_WIDGET(gtk_widget_get_root(self->priv->client_widget));
  // /* Translates the given point in client_window coordinates to coordinates
  //    relative to root coordinate system. */
  // if (!gtk_widget_compute_point(self->priv->client_widget,
  //                               root,
  //                               &GRAPHENE_POINT_INIT(area->x, area->y),
  //                               &p))
  // {
  //   graphene_point_init(&p, area->x, area->y);
  // }

  // native = gtk_widget_get_native(self->priv->client_widget);
  // /* Translates from the surface coordinates into the widget coordinates. */
  // gtk_native_get_surface_transform(native, &nx, &ny);
  // area->x = p.x + nx + tx;
  // area->y = p.y + ny + ty;
  // int scale_factor;
  // scale_factor = gtk_widget_get_scale_factor(self->priv->client_widget);
  // area->x *= scale_factor;
  // area->y *= scale_factor;
  // area->width *= scale_factor;
  // area->height *= scale_factor;

  // LOG_MESSAGE("Translated cursor location to root coordinates: x=%d y=%d, width=%d height=%d",
  //             area->x, area->y, area->width, area->height);

  if (self->priv->ibus_context != NULL)
    {
      if (IBUS_IS_INPUT_CONTEXT(self->priv->ibus_context))
        {
          ibus_input_context_set_cursor_location(
            self->priv->ibus_context,
            area->x, area->y, area->width, area->height);
        }
      else
        {
          LOG_ERROR("Invalid IBusInputContext: not IBUS_INPUT_CONTEXT, skipping cursor location");
        }
    }

  LOG_EXIT("set_cursor_location", "");
}

static void
gtk_im_bridge_context_set_use_preedit(GtkIMContext *context,
                                      gboolean use_preedit)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);

  LOG_ENTER("set_use_preedit", "use_preedit=%s", use_preedit ? "TRUE" : "FALSE");

  if (self->priv->ibus_context != NULL)
    {
      ibus_input_context_set_capabilities(
        self->priv->ibus_context,
        use_preedit ? IBUS_CAP_PREEDIT_TEXT : 0);
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

  *str = g_strdup("");
  *attrs = pango_attr_list_new();
  if (cursor_pos) {
    *cursor_pos = 0;
  }

  if (self->priv->ibus_context != NULL)
    {
      if (self->priv->preedit_text != NULL)
        {
          g_free(*str);
          *str = g_strdup(self->priv->preedit_text);
          if (cursor_pos) {
            *cursor_pos = self->priv->preedit_cursor_pos;
          }

          /* TODO: Convert IBus attributes to Pango attributes */
        }
    }

  if (cursor_pos) {
    LOG_EXIT("get_preedit_string", "str='%s' cursor_pos=%d", *str, *cursor_pos);
  } else {
    LOG_EXIT("get_preedit_string", "str='%s', cursor_pos=NULL", *str);
  }
}

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

  if (self->priv->ibus_context != NULL)
    {
      IBusText *surrounding_text = ibus_text_new_from_static_string(text ? text : "");
      ibus_input_context_set_surrounding_text(
        self->priv->ibus_context,
        surrounding_text,
        cursor_index,
        cursor_index);
      g_object_unref(surrounding_text);
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

  *text = g_strdup("");
  *cursor_index = 0;

  if (self->priv->ibus_context != NULL)
    {
      IBusText *surrounding_text = NULL;
      guint surrounding_cursor_pos = 0;

      g_object_get(self->priv->ibus_context,
                   "surrounding-text", &surrounding_text,
                   "surrounding-cursor-pos", &surrounding_cursor_pos,
                   NULL);

      if (surrounding_text != NULL)
        {
          const char *text_str = ibus_text_get_text(surrounding_text);
          g_free(*text);
          *text = g_strdup(text_str ? text_str : "");
          *cursor_index = surrounding_cursor_pos;
          g_object_unref(surrounding_text);
          result = TRUE;
        }
    }

  LOG_EXIT("get_surrounding", "result=%s text='%.32s' cursor_index=%d",
           result ? "TRUE" : "FALSE", *text, *cursor_index);
  return result;
}

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

  if (self->priv->ibus_context != NULL)
    {
      IBusText *surrounding_text = ibus_text_new_from_static_string(text ? text : "");
      ibus_input_context_set_surrounding_text(
        self->priv->ibus_context,
        surrounding_text,
        cursor_index,
        anchor_index);
      g_object_unref(surrounding_text);
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

  *text = g_strdup("");
  *cursor_index = 0;
  *anchor_index = 0;

  if (self->priv->ibus_context != NULL)
    {
      IBusText *surrounding_text = NULL;
      guint surrounding_cursor_pos = 0;

      g_object_get(self->priv->ibus_context,
                   "surrounding-text", &surrounding_text,
                   "surrounding-cursor-pos", &surrounding_cursor_pos,
                   NULL);

      if (surrounding_text != NULL)
        {
          const char *text_str = ibus_text_get_text(surrounding_text);
          g_free(*text);
          *text = g_strdup(text_str ? text_str : "");
          *cursor_index = surrounding_cursor_pos;
          *anchor_index = surrounding_cursor_pos;
          g_object_unref(surrounding_text);
          result = TRUE;
        }
    }

  LOG_EXIT("get_surrounding_with_selection",
           "result=%s text='%.32s' cursor=%d anchor=%d",
           result ? "TRUE" : "FALSE", *text, *cursor_index, *anchor_index);
  return result;
}

static gboolean
gtk_im_bridge_context_delete_surrounding(GtkIMContext *context,
                                         int offset,
                                         int n_chars)
{
  GtkImBridgeContext *self = GTK_IM_BRIDGE_CONTEXT(context);
  gboolean success = FALSE;

  LOG_ENTER("delete_surrounding", "offset=%d n_chars=%d", offset, n_chars);

  if (self->priv->ibus_context != NULL)
    {
      gboolean result;
      g_signal_emit_by_name(self, "delete-surrounding", offset, n_chars, &result);
      success = result;
    }

  LOG_EXIT("delete_surrounding", "success=%s", success ? "TRUE" : "FALSE");
  return success;
}

static void
gtk_im_bridge_context_activate_osk(GtkIMContext *context)
{
  LOG_ENTER("activate_osk", "");
  /* Not typically used on desktop, but we log it */
  LOG_EXIT("activate_osk", "");
}

static gboolean
gtk_im_bridge_context_activate_osk_with_event(GtkIMContext *context,
                                              GdkEvent *event)
{
  LOG_ENTER("activate_osk_with_event", "event=%p type=%d",
            (void *)event,
            event ? gdk_event_get_event_type(event) : 0);
  /* Not typically used on desktop, but we log it */
  LOG_EXIT("activate_osk_with_event", "");
  return FALSE;
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
  im_context_class->set_surrounding = gtk_im_bridge_context_set_surrounding;
  im_context_class->get_surrounding = gtk_im_bridge_context_get_surrounding;
  im_context_class->set_surrounding_with_selection = gtk_im_bridge_context_set_surrounding_with_selection;
  im_context_class->get_surrounding_with_selection = gtk_im_bridge_context_get_surrounding_with_selection;
  im_context_class->delete_surrounding = gtk_im_bridge_context_delete_surrounding;
  im_context_class->activate_osk = gtk_im_bridge_context_activate_osk;
  im_context_class->activate_osk_with_event = gtk_im_bridge_context_activate_osk_with_event;
}

/* ==============================================
   Public API
   ============================================== */

GtkIMContext *
gtk_im_bridge_context_new(void)
{
  return g_object_new(GTK_IM_BRIDGE_TYPE_CONTEXT, NULL);
}
