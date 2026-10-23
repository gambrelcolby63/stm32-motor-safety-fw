#ifndef MC_APP_H
#define MC_APP_H

#include "mc/types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mc_app_init(void);
/* One pass of the foreground loop: run due control ticks and kick the watchdog. */
void mc_runtime_poll(void);

/* Introspection used by host tests and the status line. */
mc_state_t mc_app_state(void);
mc_act_t mc_app_actuation(void);
mc_fault_t mc_app_fault(void);
bool mc_app_homed(void);

#ifdef __cplusplus
}
#endif

#endif /* MC_APP_H */
