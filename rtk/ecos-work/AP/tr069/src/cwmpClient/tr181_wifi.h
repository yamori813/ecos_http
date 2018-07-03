#ifndef _TR181_WIFI_H_
#define _TR181_WIFI_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tWifiLeaf[];

int getWifiInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_WIFI_H_*/
