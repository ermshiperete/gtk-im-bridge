#include "ibus-setup.h"
#include "logging.h"

static IBusBus *ibus_bus = NULL;
static IBusInputContext *ibus_context = NULL;

gboolean
ibus_setup_init(void)
{
  LOG_ENTER("ibus_setup_init", "");
  
  ibus_init();
  
  if (ibus_bus == NULL)
    {
      ibus_bus = ibus_bus_new();
      if (!ibus_bus_is_connected(ibus_bus))
        {
          LOG_ERROR("Cannot connect to IBus daemon");
          g_object_unref(ibus_bus);
          ibus_bus = NULL;
          LOG_EXIT("ibus_setup_init", "FALSE");
          return FALSE;
        }
      LOG_SIGNAL("ibus-connected", "bus is connected");
    }
  
  LOG_EXIT("ibus_setup_init", "TRUE");
  return TRUE;
}

IBusInputContext *
ibus_setup_get_context(void)
{
  LOG_ENTER("ibus_setup_get_context", "");
  
  if (ibus_bus == NULL)
    {
      if (!ibus_setup_init())
        {
          LOG_EXIT("ibus_setup_get_context", "NULL");
          return NULL;
        }
    }
  
  if (ibus_context == NULL)
    {
      ibus_context = ibus_bus_create_input_context(ibus_bus, "gtk-im-bridge");
      if (ibus_context == NULL)
        {
          LOG_ERROR("Failed to create IBusInputContext");
          LOG_EXIT("ibus_setup_get_context", "NULL");
          return NULL;
        }
      
      LOG_SIGNAL("ibus-context-created", "context=%p", (void *)ibus_context);
    }
  
  LOG_EXIT("ibus_setup_get_context", "context=%p", (void *)ibus_context);
  return ibus_context;
}

void
ibus_setup_cleanup(void)
{
  LOG_ENTER("ibus_setup_cleanup", "");
  
  if (ibus_context != NULL)
    {
      g_object_unref(ibus_context);
      ibus_context = NULL;
    }
  
  if (ibus_bus != NULL)
    {
      g_object_unref(ibus_bus);
      ibus_bus = NULL;
    }
  
  LOG_EXIT("ibus_setup_cleanup", "");
}
