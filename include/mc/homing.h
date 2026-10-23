#ifndef MC_HOMING_H
#define MC_HOMING_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MC_HOME_IDLE = 0,
    MC_HOME_SEEK,
    MC_HOME_BACKOFF,
    MC_HOME_LEAVE,
    MC_HOME_INDEX,
    MC_HOME_DONE,
    MC_HOME_FAIL
} mc_home_phase_t;

typedef struct {
    mc_home_phase_t phase;
    uint32_t ticks_in_phase;
    bool use_index;
    int which; /* 1 = tilt, 2 = pan, 3 = done */
    bool entered;
    bool done;
    bool failed;
} mc_homing_t;

typedef struct {
    bool tilt_home;
    bool tilt_index;
    bool pan_home;
    bool pan_index;
    float seek_dps;
    float creep_dps;
    uint32_t timeout_ticks;
} mc_home_sense_t;

typedef struct {
    float pan_rate_dps;
    float tilt_rate_dps;
    bool zero_pan;
    bool zero_tilt;
    bool done;
    bool failed;
    mc_home_phase_t phase;
    int which;
} mc_home_out_t;

void mc_homing_start(mc_homing_t *h, bool use_index);
void mc_homing_step(mc_homing_t *h, const mc_home_sense_t *sense, mc_home_out_t *out);
const char *mc_home_phase_name(mc_home_phase_t phase, int which);

#ifdef __cplusplus
}
#endif

#endif /* MC_HOMING_H */
