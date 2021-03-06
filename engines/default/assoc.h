/*
 * arcus-memcached - Arcus memory cache server
 * Copyright 2010-2014 NAVER Corp.
 * Copyright 2015-2016 JaM2in Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ASSOC_H
#define ASSOC_H

#include <memcached/types.h>

struct _prefix_t {
    uint8_t  nprefix;  /* the length of prefix name */
    uint8_t  internal; /* is internal prefix ? 1 or 0 */
    uint16_t dummy16;

    rel_time_t oldest_live;
    time_t     create_time;

    struct _prefix_t *h_next;  /* prefix hash chain */
    struct _prefix_t *parent_prefix;

    /* lower prefix count */
    uint32_t prefix_items;

    /* the count and bytes of cache items per item type */
    uint64_t items_count[ITEM_TYPE_MAX];
    uint64_t items_bytes[ITEM_TYPE_MAX];
    uint64_t total_count_exclusive;
    uint64_t total_bytes_exclusive;
    //uint64_t total_count_inclusive; /* NOT yet used */
    //uint64_t total_bytes_inclusive; /* NOT yet used */
};

#define PREFIX_IS_RSVD(pfx,npfx) ((npfx) == 5 && strncmp((pfx), "arcus", 5) == 0)
#define PREFIX_IS_USER(pfx,npfx) ((npfx) != 5 || strncmp((pfx), "arcus", 5) != 0)

struct bucket_info {
    uint16_t refcount; /* reference count */
    uint16_t curpower; /* current hash power:
                        * how may hash tables each hash bucket use ? (power of 2)
                        */
};

struct assoc {
    uint32_t hashpower; /* how many hash buckets in a hash table ? (power of 2) */
    uint32_t hashsize;  /* hash table size */
    uint32_t hashmask;  /* hash bucket mask */
    uint32_t rootpower; /* how many hash tables we use ? (power of 2) */

    /* cache item hash table : an array of hash tables */
    struct table {
       hash_item** hashtable;
    } *roottable;

    /* bucket info table */
    struct bucket_info *infotable;

    /* prefix hash table : single hash table */
    prefix_t**  prefix_hashtable;
    prefix_t    noprefix_stats;

    /* Number of items in the hash table. */
    unsigned int hash_items;
    unsigned int tot_prefix_items;
};

/* assoc scan structure */
struct assoc_scan {
    struct default_engine *engine;
    int        hashsz;    /* hash table size */
    int        bucket;    /* current bucket index */
    int        tabcnt;    /* table count in the bucket */
    int        tabidx;    /* table index in the bucket */
    hash_item  ph_item;   /* placeholder item itself */
    bool       ph_linked; /* placeholder item linked */
    bool       initialized;
};

/* associative array */
ENGINE_ERROR_CODE assoc_init(struct default_engine *engine);
void              assoc_final(struct default_engine *engine);

hash_item *       assoc_find(struct default_engine *engine, uint32_t hash,
                             const char *key, const size_t nkey);
int               assoc_insert(struct default_engine *engine, uint32_t hash, hash_item *item);
void              assoc_delete(struct default_engine *engine, uint32_t hash,
                               const char *key, const size_t nkey);
/* assoc scan functions */
void              assoc_scan_init(struct default_engine *engine, struct assoc_scan *scan);
int               assoc_scan_next(struct assoc_scan *scan,
                                  hash_item **item_array, int array_size);
void              assoc_scan_final(struct assoc_scan *scan);

/* prefix functions */
prefix_t *        assoc_prefix_find(struct default_engine *engine, uint32_t hash,
                                    const char *prefix, const int nprefix);
bool              assoc_prefix_isvalid(struct default_engine *engine,
                                       hash_item *it, rel_time_t current_time);
void              assoc_prefix_update_size(prefix_t *pt, ENGINE_ITEM_TYPE item_type,
                                    const size_t item_size, const bool increment);
ENGINE_ERROR_CODE assoc_prefix_link(struct default_engine *engine,
                                    hash_item *it, const size_t item_size);
void              assoc_prefix_unlink(struct default_engine *engine, hash_item *it,
                                    const size_t item_size, bool drop_if_empty);
ENGINE_ERROR_CODE assoc_get_prefix_stats(struct default_engine *engine,
                                    const char *prefix, const int nprefix,
                                    void *prefix_data);
#endif
