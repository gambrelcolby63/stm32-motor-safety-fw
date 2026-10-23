#include "mc/shell.h"

#include "mc/platform.h"

#include <string.h>

static char s_line[160];
static unsigned s_len;

void mc_shell_reset(void)
{
    s_len = 0u;
    s_line[0] = '\0';
}

int mc_shell_next(char *line, size_t cap)
{
    uint8_t byte;
    if (line == NULL || cap < 2u) {
        return 0;
    }
    while (mc_uart_read(&byte) != 0) {
        if (byte == (uint8_t)'\r') {
            continue;
        }
        if (byte == (uint8_t)'\n') {
            s_line[s_len] = '\0';
            if ((s_len + 1u) > cap) {
                s_len = 0u;
                return 0;
            }
            memcpy(line, s_line, s_len + 1u);
            s_len = 0u;
            return 1;
        }
        if ((s_len + 1u) < sizeof(s_line)) {
            s_line[s_len++] = (char)byte;
        } else {
            s_len = 0u;
        }
    }
    return 0;
}
