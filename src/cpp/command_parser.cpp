#include "mc/command.h"

#include <stdlib.h>
#include <string.h>

namespace {

const char *skip(const char *s)
{
    while (*s == ' ' || *s == '\t') {
        ++s;
    }
    return s;
}

bool take_word(const char *&s, char *word, unsigned cap)
{
    s = skip(s);
    if (*s == '\0') {
        return false;
    }
    unsigned n = 0;
    while (*s != '\0' && *s != ' ' && *s != '\t') {
        if (n + 1u < cap) {
            word[n++] = *s;
        }
        ++s;
    }
    word[n] = '\0';
    return n > 0;
}

bool take_float(const char *&s, float &out)
{
    s = skip(s);
    if (*s == '\0') {
        return false;
    }
    char buf[48];
    unsigned n = 0;
    while (*s != '\0' && *s != ' ' && *s != '\t') {
        if (n + 1u < sizeof(buf)) {
            buf[n++] = *s;
        }
        ++s;
    }
    buf[n] = '\0';
    char *end = nullptr;
    out = strtof(buf, &end);
    return end != buf && *end == '\0';
}

bool same(const char *word, const char *lit)
{
    return strcmp(word, lit) == 0;
}

bool parse_axis(const char *word, int &axis)
{
    if (same(word, "pan")) {
        axis = 0;
        return true;
    }
    if (same(word, "tilt")) {
        axis = 1;
        return true;
    }
    return false;
}

bool parse_loop(const char *word, int &loop)
{
    if (same(word, "pos")) {
        loop = 0;
        return true;
    }
    if (same(word, "rate")) {
        loop = 1;
        return true;
    }
    return false;
}

}  // namespace

extern "C" int mc_parse_line(const char *line, mc_parsed_cmd_t *out)
{
    if (out == nullptr) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->verb = MC_VERB_NONE;
    out->axis = -1;
    out->loop = -1;
    if (line == nullptr) {
        return 0;
    }

    const char *s = line;
    char word[24];
    if (!take_word(s, word, sizeof(word))) {
        return 0;
    }

    bool parsed = false;
    if (same(word, "help")) {
        out->verb = MC_VERB_HELP;
        parsed = true;
    } else if (same(word, "status")) {
        out->verb = MC_VERB_STATUS;
        parsed = true;
    } else if (same(word, "home")) {
        out->verb = MC_VERB_HOME;
        parsed = true;
    } else if (same(word, "enable")) {
        out->verb = MC_VERB_ENABLE;
        parsed = true;
    } else if (same(word, "disable")) {
        out->verb = MC_VERB_DISABLE;
        parsed = true;
    } else if (same(word, "stop")) {
        out->verb = MC_VERB_STOP;
        parsed = true;
    } else if (same(word, "reset")) {
        out->verb = MC_VERB_RESET;
        parsed = true;
    } else if (same(word, "hb")) {
        out->verb = MC_VERB_HB;
        parsed = true;
    } else if (same(word, "pos")) {
        out->verb = MC_VERB_POS;
        parsed = take_float(s, out->a) && take_float(s, out->b);
    } else if (same(word, "rate")) {
        out->verb = MC_VERB_RATE;
        parsed = take_float(s, out->a) && take_float(s, out->b);
    } else if (same(word, "mode")) {
        char which[16];
        char how[16];
        out->verb = MC_VERB_MODE;
        parsed = take_word(s, which, sizeof(which)) && take_word(s, how, sizeof(how)) && same(which, "pan");
        if (parsed && same(how, "continuous")) {
            out->flag = 1;
        } else if (parsed && same(how, "limited")) {
            out->flag = 0;
        } else {
            parsed = false;
        }
    } else if (same(word, "limits")) {
        char which[16];
        out->verb = MC_VERB_LIMITS;
        parsed = take_word(s, which, sizeof(which)) && parse_axis(which, out->axis) &&
                 take_float(s, out->a) && take_float(s, out->b) && out->a < out->b;
    } else if (same(word, "zone")) {
        char sub[16];
        if (!take_word(s, sub, sizeof(sub))) {
            parsed = false;
        } else if (same(sub, "clear")) {
            out->verb = MC_VERB_ZONE_CLEAR;
            parsed = true;
        } else if (same(sub, "add")) {
            out->verb = MC_VERB_ZONE_ADD;
            parsed = take_float(s, out->a) && take_float(s, out->b) && take_float(s, out->c) &&
                     take_float(s, out->d);
        }
    } else if (same(word, "kp") || same(word, "ki")) {
        char loop[16];
        char axis[16];
        out->verb = same(word, "kp") ? MC_VERB_KP : MC_VERB_KI;
        parsed = take_word(s, loop, sizeof(loop)) && parse_loop(loop, out->loop) &&
                 take_word(s, axis, sizeof(axis)) && parse_axis(axis, out->axis) && take_float(s, out->a) &&
                 out->a >= 0.f;
    } else if (same(word, "index")) {
        char how[16];
        out->verb = MC_VERB_INDEX;
        parsed = take_word(s, how, sizeof(how));
        if (parsed && same(how, "on")) {
            out->flag = 1;
        } else if (parsed && same(how, "off")) {
            out->flag = 0;
        } else {
            parsed = false;
        }
    }

    s = skip(s);
    if (!parsed || *s != '\0') {
        out->verb = MC_VERB_NONE;
        out->ok = 0;
        return 0;
    }
    out->ok = 1;
    return 1;
}
