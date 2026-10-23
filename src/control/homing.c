#include "mc/homing.h"

#include <string.h>

void mc_homing_start(mc_homing_t *h, bool use_index)
{
    memset(h, 0, sizeof(*h));
    h->use_index = use_index;
    h->which = 1;
    h->phase = MC_HOME_SEEK;
}

static void set_rate(mc_home_out_t *out, int which, float rate)
{
    if (which == 1) {
        out->tilt_rate_dps = rate;
    } else if (which == 2) {
        out->pan_rate_dps = rate;
    }
}

static void finish_axis(mc_homing_t *h, mc_home_out_t *out)
{
    if (h->which == 1) {
        out->zero_tilt = true;
        h->which = 2;
        h->phase = MC_HOME_SEEK;
        /* Seek is already the decision. A switch that closes after this is the
         * home edge, not a switch that was closed when the axis started. */
        h->entered = true;
        h->ticks_in_phase = 0;
    } else {
        out->zero_pan = true;
        h->which = 3;
        h->phase = MC_HOME_DONE;
        h->done = true;
        out->done = true;
    }
}

void mc_homing_step(mc_homing_t *h, const mc_home_sense_t *sense, mc_home_out_t *out)
{
    bool sw;
    bool index;
    float seek;
    float creep;

    memset(out, 0, sizeof(*out));
    if (h->done) {
        out->done = true;
        out->phase = MC_HOME_DONE;
        out->which = 3;
        return;
    }
    if (h->failed) {
        out->failed = true;
        out->phase = MC_HOME_FAIL;
        out->which = h->which;
        return;
    }

    sw = (h->which == 1) ? sense->tilt_home : sense->pan_home;
    index = (h->which == 1) ? sense->tilt_index : sense->pan_index;

    if (!h->entered) {
        h->phase = sw ? MC_HOME_BACKOFF : MC_HOME_SEEK;
        h->entered = true;
        h->ticks_in_phase = 0;
    }

    h->ticks_in_phase++;
    if (h->ticks_in_phase > sense->timeout_ticks) {
        h->failed = true;
        h->phase = MC_HOME_FAIL;
        out->failed = true;
        out->phase = MC_HOME_FAIL;
        out->which = h->which;
        return;
    }

    seek = -sense->seek_dps;
    creep = sense->creep_dps;

    /* Change phase before choosing the velocity so the command matches the phase. */
    if ((h->phase == MC_HOME_BACKOFF) && !sw) {
        h->phase = MC_HOME_SEEK;
        h->ticks_in_phase = 0;
    } else if ((h->phase == MC_HOME_SEEK) && sw) {
        h->phase = MC_HOME_LEAVE;
        h->ticks_in_phase = 0;
    } else if ((h->phase == MC_HOME_LEAVE) && !sw) {
        if (h->use_index) {
            h->phase = MC_HOME_INDEX;
            h->ticks_in_phase = 0;
        } else {
            finish_axis(h, out);
        }
    } else if ((h->phase == MC_HOME_INDEX) && index) {
        finish_axis(h, out);
    }

    if (h->phase == MC_HOME_SEEK) {
        set_rate(out, h->which, seek);
    } else if ((h->phase == MC_HOME_BACKOFF) || (h->phase == MC_HOME_LEAVE) || (h->phase == MC_HOME_INDEX)) {
        set_rate(out, h->which, creep);
    }

    out->phase = h->phase;
    out->which = h->which;
}
