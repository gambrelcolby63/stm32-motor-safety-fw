#ifndef MC_COMMAND_H
#define MC_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MC_VERB_NONE = 0,
    MC_VERB_HELP,
    MC_VERB_STATUS,
    MC_VERB_HOME,
    MC_VERB_ENABLE,
    MC_VERB_DISABLE,
    MC_VERB_STOP,
    MC_VERB_RESET,
    MC_VERB_HB,
    MC_VERB_POS,
    MC_VERB_RATE,
    MC_VERB_MODE,
    MC_VERB_LIMITS,
    MC_VERB_ZONE_ADD,
    MC_VERB_ZONE_CLEAR,
    MC_VERB_KP,
    MC_VERB_KI,
    MC_VERB_INDEX
} mc_verb_t;

typedef struct {
    mc_verb_t verb;
    float a;
    float b;
    float c;
    float d;
    int axis; /* 0 pan, 1 tilt */
    int loop; /* 0 position, 1 rate */
    int flag; /* 1 = continuous / index on, 0 = limited / index off */
    int ok;
} mc_parsed_cmd_t;

/* Returns 1 when the line is a well-formed command. */
int mc_parse_line(const char *line, mc_parsed_cmd_t *out);

#ifdef __cplusplus
}
#endif

#endif /* MC_COMMAND_H */
