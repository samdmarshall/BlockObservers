//
//  cmap.h
//  KVO
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#ifndef KVO_cmap_h
#define KVO_cmap_h

#include <sys/types.h>

typedef void *cmap;

#if __cplusplus
extern "C" {
#endif
	
	extern cmap cmap_new(void);
	extern void cmap_free(cmap *amap);
	extern void *cmap_objectForKey(cmap *amap, void *key);
	extern void cmap_setObjectForKey(cmap *amap, void *key, void *value);
	extern size_t cmap_count(cmap *amap);
	
#if __cplusplus
};
#endif

#endif
