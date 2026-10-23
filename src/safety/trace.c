#include "mc/trace.h"

#if defined(MC_HOST_BUILD)

#include <stdio.h>
#include <string.h>

static const char *const k_req_ids[] = {
    "SR-ESTOP-1",
    "SR-ESTOP-2",
    "SR-ESTOP-3",
    "SR-ESTOP-4",
    "SR-OC-1",
    "SR-OT-1",
    "SR-ENC-1",
    "SR-STALL-1",
    "SR-LIM-1",
    "SR-LIM-2",
    "SR-KO-1",
    "SR-KO-2",
    "SR-HOME-1",
    "SR-HOME-2",
    "SR-HOME-3",
    "SR-HB-1",
    "SR-WDG-1",
    "SR-WDG-2",
    "SR-RST-1",
    "SR-RST-2",
    "SR-BRK-1",
    "SR-MOVE-1",
    "SR-RATE-1",
    "SR-PAN-1",
};

#define MC_REQ_COUNT (sizeof(k_req_ids) / sizeof(k_req_ids[0]))

static unsigned char k_hit[MC_REQ_COUNT];

void mc_trace_hit(const char *id)
{
    size_t i;
    if (id == NULL) {
        return;
    }
    for (i = 0; i < MC_REQ_COUNT; i++) {
        if (strcmp(k_req_ids[i], id) == 0) {
            k_hit[i] = 1u;
            return;
        }
    }
    fprintf(stderr, "unknown safety requirement id: %s\n", id);
}

int mc_trace_missing(char *buf, size_t len)
{
    size_t i;
    size_t used = 0;
    int missing = 0;
    if (buf == NULL || len == 0) {
        return -1;
    }
    buf[0] = '\0';
    for (i = 0; i < MC_REQ_COUNT; i++) {
        size_t n;
        if (k_hit[i]) {
            continue;
        }
        n = strlen(k_req_ids[i]);
        if (used != 0 && used + 1 < len) {
            buf[used++] = ' ';
            buf[used] = '\0';
        }
        if (used + n + 1 < len) {
            memcpy(buf + used, k_req_ids[i], n + 1);
            used += n;
        }
        missing++;
    }
    return missing;
}

#endif /* MC_HOST_BUILD */
