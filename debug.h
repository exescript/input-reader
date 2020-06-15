#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

#define str(code) #code

#ifndef MIN

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#endif /* MIN */

#define INIT_ELEM(idx,value) \
	[idx] = value

#define INIT_STRVAL(val) INIT_ELEM(val, str(val))

#define MAX_STR_LEN 256

#define TAG_NAME "InputReader"

typedef char str_t[MAX_STR_LEN];

#define str_len(s) (MIN(strlen(s), MAX_STR_LEN))

#define str_equal(str1, str2) \
	((str_len(str1) == str_len(str2)) && (!strncmp(str1, str2, MAX_STR_LEN))) 

#define TAG_INFO "[ info ] "
#define TAG_WARN "[ warn ] "
#define TAG_ERR "[ error ] "

#ifndef glue
#define xglue(x, y) x ## y
#define glue(x, y) xglue(x, y)
#endif

void ev_type_str(struct input_event *, char *out);
void ev_code_str(struct input_event *, char *out);
void ev_value_str(struct input_event *, char *out);

void str_set_ch(char *s, unsigned char c);
void str_set_int(char *s, int val);
void str_set_str(char *s, const char *val);

/* #define str_set(s, val) _Generic((val),  \
 	char*	:	(str_set_str)(s, val), \
 	int		:	(str_set_int)(s, val))
*/

#define DBG_LVL_DEBUG DBG_LVL_INFO
#define DBG_LVL_RELEASE DBG_LVL_ERR

#define dbg_info(runtime, fmt, ...) \
	runtime_msg(runtime, DBG_LVL_INFO, TAG_INFO, fmt, ##__VA_ARGS__)

#define dbg_warn(runtime, fmt, ...) \
	runtime_msg(runtime, DBG_LVL_WARN, TAG_WARN, fmt, ##__VA_ARGS__)

#define dbg_err(runtime, fmt, ...) \
	runtime_msg(runtime, DBG_LVL_ERR, TAG_ERR, fmt, ##__VA_ARGS__)

typedef enum {
	DBG_LVL_NONE,
	DBG_LVL_ERR,
	DBG_LVL_WARN,
	DBG_LVL_INFO
} dbg_lvl_t;


#endif /* __DEBUG_H__ */