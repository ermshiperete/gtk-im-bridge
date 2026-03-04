#ifndef __IBUS_SETUP_H__
#define __IBUS_SETUP_H__

#include <ibus.h>

/**
 * Initialize the IBus connection. Must be called once at startup.
 * Returns TRUE on success, FALSE on failure.
 */
gboolean ibus_setup_init(void);

/**
 * Get the cached IBusInputContext if available, or create a new one.
 * Returns NULL if IBus daemon is not connected.
 */
IBusInputContext *ibus_setup_get_context(void);

/**
 * Cleanup IBus resources. Should be called during module exit.
 */
void ibus_setup_cleanup(void);

#endif // __IBUS_SETUP_H__
