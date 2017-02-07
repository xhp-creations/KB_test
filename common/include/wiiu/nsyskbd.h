#pragma once

#include <wiiu/types.h>

#ifdef __cplusplus
extern "C" {
#endif

char KBDSetup(void *connection_callback, void *disconnection_callback, void *key_callback);
char KBDTeardown();

#ifdef __cplusplus
}
#endif
