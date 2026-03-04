#ifndef __IM_BRIDGE_H__
#define __IM_BRIDGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_IM_BRIDGE_TYPE_CONTEXT (gtk_im_bridge_context_get_type())

G_DECLARE_FINAL_TYPE(GtkImBridgeContext, gtk_im_bridge_context, GTK_IM_BRIDGE, CONTEXT, GtkIMContext)

/**
 * Create a new GTK-IM-Bridge context instance.
 */
GtkIMContext *gtk_im_bridge_context_new(void);

G_END_DECLS

#endif // __IM_BRIDGE_H__
