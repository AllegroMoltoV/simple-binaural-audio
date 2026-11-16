#pragma once

// Implemented in the static library and exported by plugin-entry.cpp, as clap-wrapper's clap-first layout expects.
bool sbn_entry_init(const char *pluginPath);
void sbn_entry_deinit(void);
const void *sbn_entry_get_factory(const char *factoryId);
