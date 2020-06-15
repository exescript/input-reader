#include <stdlib.h>

#include "debug.h"
#include "runtime.h"

#define EV_MSG(status) \
	[status] = #status

static const char *ev_types[] = {
	INIT_STRVAL(EV_SYN),
	INIT_STRVAL(EV_KEY),
	INIT_STRVAL(EV_REL),
	INIT_STRVAL(EV_ABS),
	INIT_STRVAL(EV_MSC),
	INIT_STRVAL(EV_SW),
	0,
	INIT_STRVAL(EV_LED),
	INIT_STRVAL(EV_SND),
	0,
	INIT_STRVAL(EV_REP),
	INIT_STRVAL(EV_FF),
	INIT_STRVAL(EV_PWR),
	INIT_STRVAL(EV_FF_STATUS),
	0,
	INIT_STRVAL(EV_MAX),
};

void str_set_ch(char *s, unsigned char c) {
	memset(s, c, MAX_STR_LEN);
}

void str_set_int(char *s, int val) {
	str_set_ch(s, (unsigned char) val);
}

void str_set_str(char *s, const char *val) {
	strncpy(s, val, MIN(str_len(val), MAX_STR_LEN));
}

void ev_type_str(struct input_event *event, char *out) {
	if (event->type >= EV_MAX)
		snprintf(out, MAX_STR_LEN, "%s", ev_types[EV_MAX]);
	else
		snprintf(out, MAX_STR_LEN, "%s", ev_types[event->type]);
}

void ev_code_str(struct input_event *event, char *out) {
	snprintf(out, MAX_STR_LEN, "%hu", event->value);
}

void ev_value_str(struct input_event *event, char *out) {
	snprintf(out, MAX_STR_LEN, "%u", event->value);
}
