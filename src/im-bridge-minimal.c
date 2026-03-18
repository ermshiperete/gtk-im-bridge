#include "im-bridge.h"
#include "logging.h"

struct _GtkImBridgeContext
{
  GtkIMContext parent_instance;
};

G_DEFINE_TYPE(GtkImBridgeContext,
              gtk_im_bridge_context,
              GTK_TYPE_IM_CONTEXT)

/* Virtual method implementations */

static void
gtk_im_bridge_context_set_client_window(GtkIMContext *context,
                                        GdkWindow *window)
{
  // LOG_ENTER(__FUNCTION__, "window=%p", (void *)window);
  LOG_ENTER(__FUNCTION__, "");
  LOG_EXIT(__FUNCTION__, "");
}

static gboolean
gtk_im_bridge_context_filter_keypress(GtkIMContext *context,
                                      GdkEventKey *event)
{
  LOG_ENTER(__FUNCTION__, "event=%p", (void *)event);
  LOG_EXIT(__FUNCTION__, "result=FALSE");
  return FALSE;
}

static void
gtk_im_bridge_context_focus_in(GtkIMContext *context)
{
  LOG_ENTER(__FUNCTION__, "");
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_focus_out(GtkIMContext *context)
{
  LOG_ENTER(__FUNCTION__, "");
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_reset(GtkIMContext *context)
{
  LOG_ENTER(__FUNCTION__, "");
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_set_cursor_location(GtkIMContext *context,
                                          GdkRectangle *area)
{
  LOG_ENTER(__FUNCTION__, "area={x=%d y=%d w=%d h=%d}",
            area ? area->x : 0, area ? area->y : 0,
            area ? area->width : 0, area ? area->height : 0);
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_set_use_preedit(GtkIMContext *context,
                                      gboolean use_preedit)
{
  LOG_ENTER(__FUNCTION__, "use_preedit=%s", use_preedit ? "TRUE" : "FALSE");
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_set_surrounding(GtkIMContext *context,
                                      const char *text,
                                      int len,
                                      int cursor_index)
{
  LOG_ENTER(__FUNCTION__, "text_len=%d cursor_index=%d", len, cursor_index);
  LOG_EXIT(__FUNCTION__, "");
}

static gboolean
gtk_im_bridge_context_get_surrounding(GtkIMContext *context,
                                      char **text,
                                      int *cursor_index)
{
  LOG_ENTER(__FUNCTION__, "");
  if (text)
    *text = g_strdup("");
  if (cursor_index)
    *cursor_index = 0;
  LOG_EXIT(__FUNCTION__, "result=TRUE");
  return TRUE;
}

static gboolean
gtk_im_bridge_context_delete_surrounding(GtkIMContext *context,
                                         int offset,
                                         int n_chars)
{
  LOG_ENTER(__FUNCTION__, "offset=%d n_chars=%d", offset, n_chars);
  LOG_EXIT(__FUNCTION__, "result=FALSE");
  return FALSE;
}

static void
gtk_im_bridge_context_finalize(GObject *object)
{
  LOG_ENTER(__FUNCTION__, "");
  G_OBJECT_CLASS(gtk_im_bridge_context_parent_class)->finalize(object);
  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_class_init(GtkImBridgeContextClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(klass);

  LOG_ENTER(__FUNCTION__, "");

  gobject_class->finalize = gtk_im_bridge_context_finalize;

  im_context_class->set_client_window = gtk_im_bridge_context_set_client_window;
  im_context_class->filter_keypress = gtk_im_bridge_context_filter_keypress;
  im_context_class->focus_in = gtk_im_bridge_context_focus_in;
  im_context_class->focus_out = gtk_im_bridge_context_focus_out;
  im_context_class->reset = gtk_im_bridge_context_reset;
  im_context_class->set_cursor_location = gtk_im_bridge_context_set_cursor_location;
  im_context_class->set_use_preedit = gtk_im_bridge_context_set_use_preedit;
  im_context_class->set_surrounding = gtk_im_bridge_context_set_surrounding;
  im_context_class->get_surrounding = gtk_im_bridge_context_get_surrounding;
  im_context_class->delete_surrounding = gtk_im_bridge_context_delete_surrounding;
  /* Use parent class methods for all IM context methods */

  LOG_EXIT(__FUNCTION__, "");
}

static void
gtk_im_bridge_context_init(GtkImBridgeContext *self)
{
  LOG_ENTER(__FUNCTION__, "");
  LOG_EXIT(__FUNCTION__, "");
}

GtkIMContext *
gtk_im_bridge_context_new(void)
{
  GtkIMContext *context;
  LOG_ENTER(__FUNCTION__, "");
  context = g_object_new(GTK_IM_BRIDGE_TYPE_CONTEXT, NULL);
  LOG_EXIT(__FUNCTION__, "context=%p", (void *)context);
  return context;
}
