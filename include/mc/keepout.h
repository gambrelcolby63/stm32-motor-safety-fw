#ifndef MC_KEEPOUT_H
#define MC_KEEPOUT_H

#include "mc/config.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float pan_min;
    float pan_max;
    float tilt_min;
    float tilt_max;
    bool used;
} mc_zone_t;

typedef struct {
    mc_zone_t zones[MC_ZONE_MAX];
} mc_keepout_t;

void mc_keepout_clear(mc_keepout_t *k);
/* Returns false if the table is full or the rectangle is empty. */
bool mc_keepout_add(mc_keepout_t *k, float pan_min, float pan_max, float tilt_min, float tilt_max);
bool mc_keepout_contains(const mc_keepout_t *k, float pan, float tilt);
/* True if the straight segment in pan/tilt space touches any zone.
 * pan_wrap splits a segment that crosses the ±180 branch cut. */
bool mc_keepout_segment_hits(const mc_keepout_t *k,
                             float pan0, float tilt0,
                             float pan1, float tilt1,
                             bool pan_wrap);

#ifdef __cplusplus
}
#endif

#endif /* MC_KEEPOUT_H */
