#include "mc/app.h"

int main(void)
{
    mc_app_init();
    for (;;) {
        mc_runtime_poll();
        __asm volatile("wfi");
    }
}
