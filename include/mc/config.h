#ifndef MC_CONFIG_H
#define MC_CONFIG_H

/*
 * Tunable defaults for the pan/tilt positioner.
 * Angles are degrees at the axis output. Counts are quadrature counts.
 */

#define MC_SYSCLK_HZ            16000000u
#define MC_CONTROL_HZ           1000u
#define MC_DT_S                 0.001f
#define MC_PWM_HZ               20000u
#define MC_UART_BAUD            115200u

#define MC_COUNTS_PER_REV       4096.f
#define MC_DEG_PER_COUNT        (360.f / MC_COUNTS_PER_REV)

#define MC_PAN_V_MAX_DPS        120.f
#define MC_TILT_V_MAX_DPS       80.f
#define MC_PAN_A_MAX            360.f
#define MC_TILT_A_MAX           280.f

#define MC_PAN_SOFT_MIN_DEG     (-170.f)
#define MC_PAN_SOFT_MAX_DEG     (170.f)
#define MC_TILT_SOFT_MIN_DEG    (-45.f)
#define MC_TILT_SOFT_MAX_DEG    (90.f)

#define MC_CURRENT_LIMIT_A      8.f
#define MC_TEMP_LIMIT_C         80.f
#define MC_ADC_VREF             3.3f
#define MC_I_SENSE_V_PER_A      0.132f   /* shunt * amplifier, midscale = 0 A */
#define MC_TEMP_V_PER_C         0.01f    /* linear sensor, 0 V = 0 C */

#define MC_HEARTBEAT_MS         500u
#define MC_STALL_MS             1000u
#define MC_STALL_DUTY_PCT       25.f
#define MC_STALL_RATE_DPS       5.f
#define MC_BRAKE_TIMEOUT_MS     400u
#define MC_BRAKE_HOLD_MS        30u

#define MC_HOME_SEEK_DPS        30.f
#define MC_HOME_CREEP_DPS       8.f
#define MC_HOME_TIMEOUT_MS      8000u

#define MC_POS_TOL_DEG          0.15f
#define MC_RATE_TOL_DPS         1.0f
#define MC_ENC_MAX_DELTA        250      /* counts per ms; above this is a glitch */

#define MC_ZONE_MAX             4

#define MC_POS_KP_DEFAULT       3.0f
#define MC_POS_KI_DEFAULT       0.4f
#define MC_RATE_KP_DEFAULT      0.45f
#define MC_RATE_KI_DEFAULT      1.2f

#endif /* MC_CONFIG_H */
