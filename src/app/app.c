#include "mc/app.h"

#include "mc/analog.h"
#include "mc/axis.h"
#include "mc/command.h"
#include "mc/config.h"
#include "mc/homing.h"
#include "mc/keepout.h"
#include "mc/platform.h"
#include "mc/safety.h"
#include "mc/shell.h"
#include "mc/trajectory.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    mc_sm_t sm;
    mc_axis_t pan;
    mc_axis_t tilt;
    mc_keepout_t keepout;
    mc_homing_t homing;
    mc_enc_track_t pan_enc;
    mc_enc_track_t tilt_enc;
    int32_t pan_zero;
    int32_t tilt_zero;
    float pan_deg;
    float tilt_deg;
    float pan_rate;
    float tilt_rate;
    bool have_angle;
    float pan_amps;
    float tilt_amps;
    float temp_c;
    bool use_index;
    uint32_t last_hb_ms;
    bool req_home;
    bool req_enable;
    bool req_disable;
    bool req_stop;
    bool req_reset;
    bool req_move;
    int move_kind;
    float cmd_a;
    float cmd_b;
    bool home_done_pulse;
    bool home_fail_pulse;
    bool homing_active;
    float home_pan_rate;
    float home_tilt_rate;
    mc_home_phase_t phase;
    int phase_which;
    uint32_t brake_ms;
    uint32_t pan_stall_ms;
    uint32_t tilt_stall_ms;
    char phase_name[20];
    bool logged_once;
    mc_state_t logged_state;
    mc_act_t logged_act;
    mc_fault_t logged_fault;
    bool logged_homed;
    char logged_phase[20];
    bool self_test_ok;
    bool watchdog_boot;
} mc_app_t;

static mc_app_t g;

static void add_str(char *buf, size_t cap, size_t *n, const char *text)
{
    while ((*text != '\0') && ((*n + 1u) < cap)) {
        buf[*n] = *text++;
        *n += 1u;
    }
    if (*n < cap) {
        buf[*n] = '\0';
    }
}

static void add_f2(char *buf, size_t cap, size_t *n, float value)
{
    char tmp[24];
    int whole;
    int frac;
    int sign = 0;
    if (!isfinite(value)) {
        value = 0.f;
    }
    if (value < 0.f) {
        sign = 1;
        value = -value;
    }
    whole = (int)value;
    frac = (int)(((value - (float)whole) * 100.f) + 0.5f);
    if (frac >= 100) {
        whole += 1;
        frac -= 100;
    }
    if (sign != 0) {
        (void)snprintf(tmp, sizeof(tmp), "-%d.%02d", whole, frac);
    } else {
        (void)snprintf(tmp, sizeof(tmp), "%d.%02d", whole, frac);
    }
    add_str(buf, cap, n, tmp);
}

static void format_status(char *buf, size_t cap, const char *prefix)
{
    size_t n = 0u;
    buf[0] = '\0';
    add_str(buf, cap, &n, prefix);
    add_str(buf, cap, &n, " state=");
    add_str(buf, cap, &n, mc_state_name(g.sm.state));
    add_str(buf, cap, &n, " act=");
    add_str(buf, cap, &n, mc_act_name(g.sm.act));
    add_str(buf, cap, &n, " homed=");
    add_str(buf, cap, &n, g.sm.homed ? "1" : "0");
    add_str(buf, cap, &n, " fault=");
    add_str(buf, cap, &n, mc_fault_name(g.sm.fault));
    add_str(buf, cap, &n, " phase=");
    add_str(buf, cap, &n, g.phase_name);
    add_str(buf, cap, &n, " pan=");
    add_f2(buf, cap, &n, g.pan_deg);
    add_str(buf, cap, &n, " tilt=");
    add_f2(buf, cap, &n, g.tilt_deg);
    add_str(buf, cap, &n, "\n");
}

static void reply(const char *text)
{
    mc_uart_write(text);
}

static void refresh_phase_name(void)
{
    const char *name = mc_home_phase_name(g.phase, g.phase_which);
    (void)snprintf(g.phase_name, sizeof(g.phase_name), "%s", name);
}

static bool pose_rejected(float pan, float tilt, const char **why)
{
    if (!g.pan.wrap) {
        if ((pan < g.pan.soft_min) || (pan > g.pan.soft_max) || (tilt < g.tilt.soft_min) ||
            (tilt > g.tilt.soft_max)) {
            *why = "ERR limit\n";
            return true;
        }
    } else if ((tilt < g.tilt.soft_min) || (tilt > g.tilt.soft_max)) {
        *why = "ERR limit\n";
        return true;
    }
    if (mc_keepout_contains(&g.keepout, pan, tilt) ||
        mc_keepout_segment_hits(&g.keepout, g.pan_deg, g.tilt_deg, pan, tilt, g.pan.wrap)) {
        *why = "ERR keep-out\n";
        return true;
    }
    return false;
}

static float stopping_pose(float pos, float rate, float accel, bool wrap)
{
    float dist;
    if (accel < 1.f) {
        accel = 1.f;
    }
    dist = (rate * rate) / (2.f * accel);
    if (rate < 0.f) {
        dist = -dist;
    }
    pos += dist;
    if (wrap) {
        pos = mc_wrap180(pos);
    }
    return pos;
}

static void handle_line(const char *line)
{
    mc_parsed_cmd_t cmd;
    const char *why = "ERR state\n";
    char status[192];

    if (mc_parse_line(line, &cmd) == 0) {
        reply("ERR parse\n");
        return;
    }

    switch (cmd.verb) {
    case MC_VERB_HELP:
        reply("OK cmds: help status home enable disable stop reset hb pos rate mode limits zone kp ki index\n");
        g.last_hb_ms = mc_millis();
        break;
    case MC_VERB_STATUS:
        format_status(status, sizeof(status), "OK");
        reply(status);
        g.last_hb_ms = mc_millis();
        break;
    case MC_VERB_HB:
        g.last_hb_ms = mc_millis();
        reply("OK hb\n");
        break;
    case MC_VERB_HOME:
        if (g.sm.state != MC_STATE_SAFE) {
            reply("ERR state\n");
            break;
        }
        g.req_home = true;
        g.last_hb_ms = mc_millis();
        reply("OK home\n");
        break;
    case MC_VERB_ENABLE:
        g.last_hb_ms = mc_millis();
        if (g.sm.state != MC_STATE_SAFE) {
            reply("ERR state\n");
            break;
        }
        if (!g.sm.homed) {
            reply("ERR not-homed\n"); /* SR-HOME-1 */
            break;
        }
        g.req_enable = true;
        reply("OK enable\n");
        break;
    case MC_VERB_DISABLE:
        g.req_disable = true;
        g.last_hb_ms = mc_millis();
        reply("OK disable\n");
        break;
    case MC_VERB_STOP:
        g.req_stop = true;
        g.last_hb_ms = mc_millis();
        reply("OK stop\n");
        break;
    case MC_VERB_RESET:
        g.req_reset = true;
        g.last_hb_ms = mc_millis();
        reply("OK reset\n");
        break;
    case MC_VERB_POS:
        if ((g.sm.state != MC_STATE_ENABLED) && (g.sm.state != MC_STATE_MOVING)) {
            reply("ERR state\n");
            break;
        }
        if (pose_rejected(cmd.a, cmd.b, &why)) {
            reply(why); /* SR-KO-1, SR-LIM-2 command gate */
            break;
        }
        g.req_move = true;
        g.move_kind = 0;
        g.cmd_a = cmd.a;
        g.cmd_b = cmd.b;
        g.last_hb_ms = mc_millis();
        reply("OK pos\n");
        break;
    case MC_VERB_RATE: {
        float pan_stop = stopping_pose(g.pan_deg, cmd.a, g.pan.a_max, g.pan.wrap);
        float tilt_stop = stopping_pose(g.tilt_deg, cmd.b, g.tilt.a_max, false);
        if ((g.sm.state != MC_STATE_ENABLED) && (g.sm.state != MC_STATE_MOVING)) {
            reply("ERR state\n");
            break;
        }
        if (pose_rejected(pan_stop, tilt_stop, &why)) {
            reply(why);
            break;
        }
        g.req_move = true;
        g.move_kind = 1;
        g.cmd_a = cmd.a;
        g.cmd_b = cmd.b;
        g.last_hb_ms = mc_millis();
        reply("OK rate\n");
        break;
    }
    case MC_VERB_MODE:
        if ((g.sm.state != MC_STATE_SAFE) && (g.sm.state != MC_STATE_ENABLED)) {
            reply("ERR state\n");
            break;
        }
        g.pan.limited = (cmd.flag == 0);
        g.pan.wrap = (cmd.flag != 0); /* SR-PAN-1 */
        g.last_hb_ms = mc_millis();
        reply(cmd.flag != 0 ? "OK mode pan continuous\n" : "OK mode pan limited\n");
        break;
    case MC_VERB_LIMITS:
        if ((g.sm.state == MC_STATE_FAULT) || (g.sm.state == MC_STATE_ESTOP) || (g.sm.state == MC_STATE_HOMING)) {
            reply("ERR state\n");
            break;
        }
        if (cmd.axis == 0) {
            g.pan.soft_min = cmd.a;
            g.pan.soft_max = cmd.b;
        } else {
            g.tilt.soft_min = cmd.a;
            g.tilt.soft_max = cmd.b;
            g.tilt.limited = true;
        }
        g.last_hb_ms = mc_millis();
        reply("OK limits\n");
        break;
    case MC_VERB_ZONE_ADD:
        if (mc_keepout_contains(&g.keepout, g.pan_deg, g.tilt_deg)) {
            reply("ERR keep-out\n");
            break;
        }
        if ((g.pan_deg >= cmd.a) && (g.pan_deg <= cmd.b) && (g.tilt_deg >= cmd.c) && (g.tilt_deg <= cmd.d)) {
            reply("ERR keep-out\n");
            break;
        }
        if (!mc_keepout_add(&g.keepout, cmd.a, cmd.b, cmd.c, cmd.d)) {
            reply("ERR parse\n");
            break;
        }
        g.last_hb_ms = mc_millis();
        reply("OK zone\n");
        break;
    case MC_VERB_ZONE_CLEAR:
        mc_keepout_clear(&g.keepout);
        g.last_hb_ms = mc_millis();
        reply("OK zone clear\n");
        break;
    case MC_VERB_KP:
    case MC_VERB_KI: {
        mc_axis_t *axis = (cmd.axis == 0) ? &g.pan : &g.tilt;
        mc_pi_t *pi = (cmd.loop == 0) ? &axis->pos_pi : &axis->rate_pi;
        if (cmd.verb == MC_VERB_KP) {
            pi->kp = cmd.a;
        } else {
            pi->ki = cmd.a;
        }
        g.last_hb_ms = mc_millis();
        reply("OK gain\n");
        break;
    }
    case MC_VERB_INDEX:
        g.use_index = (cmd.flag != 0);
        g.last_hb_ms = mc_millis();
        reply(g.use_index ? "OK index on\n" : "OK index off\n");
        break;
    case MC_VERB_NONE:
    default:
        reply("ERR parse\n");
        break;
    }
}

static void service_shell(void)
{
    char line[160];
    while (mc_shell_next(line, sizeof(line)) != 0) {
        handle_line(line);
    }
}

static void sample_axes(void)
{
    int32_t pan_counts = mc_encoder_sample(TIM3, &g.pan_enc);
    int32_t tilt_counts = mc_encoder_sample(TIM4, &g.tilt_enc);
    float pan = (float)(pan_counts - g.pan_zero) * MC_DEG_PER_COUNT;
    float tilt = (float)(tilt_counts - g.tilt_zero) * MC_DEG_PER_COUNT;
    float pan_rate = 0.f;
    float tilt_rate = 0.f;
    if (g.pan.wrap) {
        pan = mc_wrap180(pan);
    }
    if (g.have_angle) {
        float dpan = pan - g.pan_deg;
        if (g.pan.wrap) {
            dpan = mc_wrap180(dpan);
        }
        pan_rate = dpan / MC_DT_S;
        tilt_rate = (tilt - g.tilt_deg) / MC_DT_S;
    }
    g.have_angle = true;
    g.pan_deg = pan;
    g.tilt_deg = tilt;
    g.pan_rate = pan_rate;
    g.tilt_rate = tilt_rate;
}

static void sample_analog(void)
{
    uint8_t channel = 0u;
    uint16_t raw = 0u;
    if (!mc_adc_service(&channel, &raw)) {
        return;
    }
    if (channel == 0u) {
        g.pan_amps = mc_amps_from_raw(raw);
    } else if (channel == 1u) {
        g.tilt_amps = mc_amps_from_raw(raw);
    } else if (channel == 4u) {
        g.temp_c = mc_temp_c_from_raw(raw);
    }
}

static bool axis_stalled(uint32_t *timer, float duty, float rate)
{
    if ((fabsf(duty) >= MC_STALL_DUTY_PCT) && (fabsf(rate) <= MC_STALL_RATE_DPS)) {
        *timer += 1u;
    } else {
        *timer = 0u;
    }
    return *timer >= MC_STALL_MS;
}

static void log_if_changed(void)
{
    char line[192];
    bool changed = !g.logged_once || (g.logged_state != g.sm.state) || (g.logged_act != g.sm.act) ||
                   (g.logged_fault != g.sm.fault) || (g.logged_homed != g.sm.homed) ||
                   (strcmp(g.logged_phase, g.phase_name) != 0);
    if (!changed) {
        return;
    }
    format_status(line, sizeof(line), "EVT");
    reply(line);
    g.logged_once = true;
    g.logged_state = g.sm.state;
    g.logged_act = g.sm.act;
    g.logged_fault = g.sm.fault;
    g.logged_homed = g.sm.homed;
    (void)snprintf(g.logged_phase, sizeof(g.logged_phase), "%s", g.phase_name);
}

static void mc_app_tick(void)
{
    mc_sm_in_t in;
    mc_act_t prev;
    bool energized;
    bool do_move;
    uint32_t now = mc_millis();

    sample_axes();
    sample_analog();
    service_shell();

    memset(&in, 0, sizeof(in));
    in.self_test_ok = g.self_test_ok;
    in.watchdog_reset = (g.sm.state == MC_STATE_INIT) && g.watchdog_boot;
    in.estop = mc_gpio_read(GPIOC, 0u);
    in.overcurrent = (fabsf(g.pan_amps) > MC_CURRENT_LIMIT_A) || (fabsf(g.tilt_amps) > MC_CURRENT_LIMIT_A) ||
                     mc_gpio_read(GPIOD, 0u) || mc_gpio_read(GPIOD, 1u);
    in.overtemperature = g.temp_c > MC_TEMP_LIMIT_C;
    in.encoder_fault = g.pan_enc.fault || g.tilt_enc.fault;
    in.keepout_violation = mc_keepout_contains(&g.keepout, g.pan_deg, g.tilt_deg); /* SR-KO-2 */

    prev = g.sm.act;
    energized = (prev == MC_ACT_HOLD) || (prev == MC_ACT_MOVE) || (prev == MC_ACT_BRAKE) || (prev == MC_ACT_HOME);
    if (energized) {
        bool pan_limit = (!g.pan.wrap) && (mc_gpio_read(GPIOB, 2u) || mc_gpio_read(GPIOB, 3u));
        bool tilt_limit = mc_gpio_read(GPIOB, 10u) || mc_gpio_read(GPIOB, 11u);
        in.limit_hit = pan_limit || tilt_limit; /* SR-LIM-1 */
    }
    if ((prev == MC_ACT_MOVE) || (prev == MC_ACT_HOME)) {
        bool pan_stall = axis_stalled(&g.pan_stall_ms, g.pan.duty, g.pan_rate);
        bool tilt_stall = axis_stalled(&g.tilt_stall_ms, g.tilt.duty, g.tilt_rate);
        in.stall = pan_stall || tilt_stall;
    } else {
        g.pan_stall_ms = 0u;
        g.tilt_stall_ms = 0u;
    }
    if (prev == MC_ACT_BRAKE) {
        g.brake_ms += 1u;
        if (((fabsf(g.pan_rate) <= MC_RATE_TOL_DPS) && (fabsf(g.tilt_rate) <= MC_RATE_TOL_DPS) &&
             (g.brake_ms >= MC_BRAKE_HOLD_MS)) ||
            (g.brake_ms >= MC_BRAKE_TIMEOUT_MS)) {
            in.braking_done = true;
        }
    } else {
        g.brake_ms = 0u;
    }
    if ((prev == MC_ACT_MOVE) && (g.pan.mode == MC_AXIS_MODE_TRACK) && (g.tilt.mode == MC_AXIS_MODE_TRACK) &&
        g.pan.trap.done && g.tilt.trap.done && (fabsf(g.pan_rate) <= MC_RATE_TOL_DPS) &&
        (fabsf(g.tilt_rate) <= MC_RATE_TOL_DPS)) {
        in.motion_done = true;
    }

    in.homing_done = g.home_done_pulse;
    in.homing_failed = g.home_fail_pulse;
    g.home_done_pulse = false;
    g.home_fail_pulse = false;
    in.cmd_home = g.req_home;
    in.cmd_enable = g.req_enable;
    in.cmd_disable = g.req_disable;
    in.cmd_stop = g.req_stop;
    in.cmd_reset = g.req_reset;
    do_move = g.req_move;
    in.cmd_move = g.req_move;
    g.req_home = false;
    g.req_enable = false;
    g.req_disable = false;
    g.req_stop = false;
    g.req_reset = false;
    g.req_move = false;
    in.heartbeat_ok = (uint32_t)(now - g.last_hb_ms) < MC_HEARTBEAT_MS;

    mc_safety_step(&g.sm, &in);

    if (g.sm.act == MC_ACT_HOME) {
        mc_home_sense_t sense;
        mc_home_out_t out;
        if (!g.homing_active) {
            mc_homing_start(&g.homing, g.use_index);
            g.homing_active = true;
        }
        memset(&sense, 0, sizeof(sense));
        sense.tilt_home = mc_gpio_read(GPIOB, 9u);
        sense.tilt_index = mc_gpio_read(GPIOB, 8u);
        sense.pan_home = mc_gpio_read(GPIOB, 1u);
        sense.pan_index = mc_gpio_read(GPIOB, 0u);
        sense.seek_dps = MC_HOME_SEEK_DPS;
        sense.creep_dps = MC_HOME_CREEP_DPS;
        sense.timeout_ticks = MC_HOME_TIMEOUT_MS;
        mc_homing_step(&g.homing, &sense, &out);
        g.home_pan_rate = out.pan_rate_dps;
        g.home_tilt_rate = out.tilt_rate_dps;
        g.phase = out.phase;
        g.phase_which = out.which;
        if (out.zero_tilt) {
            g.tilt_zero = g.tilt_enc.extended;
            g.tilt_deg = 0.f;
            g.tilt_rate = 0.f;
        }
        if (out.zero_pan) {
            g.pan_zero = g.pan_enc.extended;
            g.pan_deg = 0.f;
            g.pan_rate = 0.f;
        }
        if (out.done) {
            g.home_done_pulse = true;
            g.homing_active = false;
        }
        if (out.failed) {
            g.home_fail_pulse = true; /* SR-HOME-3 */
            g.homing_active = false;
        }
    } else {
        g.homing_active = false;
        if (g.sm.state != MC_STATE_HOMING) {
            g.phase = MC_HOME_IDLE;
            g.phase_which = 0;
        }
    }
    refresh_phase_name();

    mc_axis_set_measurement(&g.pan, g.pan_deg, g.pan_rate);
    mc_axis_set_measurement(&g.tilt, g.tilt_deg, g.tilt_rate);

    if (g.sm.act == MC_ACT_OFF) {
        mc_axis_off(&g.pan);
        mc_axis_off(&g.tilt);
    } else if (g.sm.act == MC_ACT_HOLD) {
        if (prev != MC_ACT_HOLD) {
            mc_axis_hold_here(&g.pan);
            mc_axis_hold_here(&g.tilt);
        }
    } else if (g.sm.act == MC_ACT_BRAKE) {
        mc_axis_command_rate(&g.pan, 0.f);
        mc_axis_command_rate(&g.tilt, 0.f);
    } else if (g.sm.act == MC_ACT_HOME) {
        mc_axis_command_rate(&g.pan, g.home_pan_rate);
        mc_axis_command_rate(&g.tilt, g.home_tilt_rate);
    } else if ((g.sm.act == MC_ACT_MOVE) && do_move) {
        if (g.move_kind == 0) {
            mc_axis_command_position(&g.pan, g.cmd_a);
            mc_axis_command_position(&g.tilt, g.cmd_b);
        } else {
            mc_axis_command_rate(&g.pan, g.cmd_a);
            mc_axis_command_rate(&g.tilt, g.cmd_b);
        }
    }

    (void)mc_axis_step(&g.pan, MC_DT_S);
    (void)mc_axis_step(&g.tilt, MC_DT_S);

    {
        bool enable = g.sm.act != MC_ACT_OFF;
        mc_motor_output(0, enable ? g.pan.duty : 0.f, enable, enable);
        mc_motor_output(1, enable ? g.tilt.duty : 0.f, enable, enable);
    }

    mc_leds_show(((now / 500u) & 1u) != 0u,
                 (g.sm.state == MC_STATE_ENABLED) || (g.sm.state == MC_STATE_MOVING) ||
                     (g.sm.state == MC_STATE_HOMING),
                 (g.sm.state == MC_STATE_FAULT) || (g.sm.state == MC_STATE_ESTOP),
                 (g.sm.state == MC_STATE_MOVING) || (g.sm.state == MC_STATE_HOMING));

    if ((g.sm.state == MC_STATE_FAULT) && (g.sm.fault == MC_FAULT_ENCODER)) {
        g.pan_enc.fault = false;
        g.tilt_enc.fault = false;
    }

    log_if_changed();
}

void mc_app_init(void)
{
    memset(&g, 0, sizeof(g));
    mc_shell_reset();
    mc_board_init();
    g.self_test_ok = mc_board_self_test_ok();
    g.watchdog_boot = mc_board_watchdog_boot();
    g.temp_c = 25.f;
    mc_safety_init(&g.sm);
    mc_axis_init(&g.pan, MC_PAN_V_MAX_DPS, MC_PAN_A_MAX, MC_PAN_SOFT_MIN_DEG, MC_PAN_SOFT_MAX_DEG, true);
    mc_axis_init(&g.tilt, MC_TILT_V_MAX_DPS, MC_TILT_A_MAX, MC_TILT_SOFT_MIN_DEG, MC_TILT_SOFT_MAX_DEG, true);
    mc_keepout_clear(&g.keepout);
    mc_encoder_track_reset(&g.pan_enc, MC_ENC_MAX_DELTA);
    mc_encoder_track_reset(&g.tilt_enc, MC_ENC_MAX_DELTA);
    g.phase = MC_HOME_IDLE;
    refresh_phase_name();
    g.last_hb_ms = mc_millis();
    reply("BOOT pan-tilt-gimbal\n");
}

void mc_runtime_poll(void)
{
    static uint32_t last_ms;
    uint32_t now = mc_millis();
    uint32_t steps = now - last_ms;
    if (steps > 20u) {
        steps = 20u;
    }
    last_ms = now;
    while (steps > 0u) {
        mc_app_tick();
        steps--;
    }
    mc_iwdg_kick(); /* SR-WDG-2: the foreground loop is what pets the watchdog. */
}

mc_state_t mc_app_state(void)
{
    return g.sm.state;
}

mc_act_t mc_app_actuation(void)
{
    return g.sm.act;
}

mc_fault_t mc_app_fault(void)
{
    return g.sm.fault;
}

bool mc_app_homed(void)
{
    return g.sm.homed;
}
