#ifndef MC_ANALOG_H
#define MC_ANALOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

float mc_amps_from_raw(uint16_t raw);
float mc_temp_c_from_raw(uint16_t raw);

#ifdef __cplusplus
}
#endif

#endif /* MC_ANALOG_H */
