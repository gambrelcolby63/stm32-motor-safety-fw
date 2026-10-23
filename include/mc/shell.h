#ifndef MC_SHELL_H
#define MC_SHELL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Pull queued UART bytes. Returns 1 and writes a NUL-terminated line when a newline arrives. */
int mc_shell_next(char *line, size_t cap);
void mc_shell_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MC_SHELL_H */
