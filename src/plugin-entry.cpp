#include <clap/clap.h>

#include "plugin-entry.h"

extern "C" {
CLAP_EXPORT const clap_plugin_entry clap_entry = {CLAP_VERSION_INIT, sbn_entry_init, sbn_entry_deinit,
						  sbn_entry_get_factory};
}
