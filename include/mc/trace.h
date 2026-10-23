#ifndef MC_TRACE_H
#define MC_TRACE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MC_HOST_BUILD)
void mc_trace_hit(const char *id);
/* Writes a space-separated list of requirement ids that no test hit.
 * Returns the number of missing ids. */
int mc_trace_missing(char *buf, size_t len);
#else
#define mc_trace_hit(id) ((void)(id))
#endif

#ifdef __cplusplus
}
#endif

#endif /* MC_TRACE_H */
