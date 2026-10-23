#ifndef MC_HOST_HW_H
#define MC_HOST_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Host-only register-mock helpers. Not part of the firmware image. */
void mc_host_hw_reset(void);
void mc_adc_host_set_channel(uint8_t channel, uint16_t raw);
void mc_adc_host_set_invalid(void);

#ifdef __cplusplus
}
#endif

#endif /* MC_HOST_HW_H */
