#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <linux/input.h>
#include <stdlib.h>
#include "debug.h"

#define MAX_PATH_LEN 256
#define MAX_TAG_LEN 256
#define MAX_STATUS_LEN 256

typedef enum {
	RUNTIME_UNINITIALIZED = 0,
	RUNTIME_OPENED,
	RUNTIME_TERMINATING,
	RUNTIME_CLOSED,
} runtime_state_t;

typedef struct shell_params {
	
} shell_params_t;

typedef enum {
	STATUS_OK = 0,
	STATUS_ERR_SHELLPARAMS,
	STATUS_ERR_NOMEM,
	STATUS_ERR_FILE,
	STATUS_ERR_UNKNOWN,
} status_t;

typedef struct runtime_opts {
	int quiet;
	int debug;
	int raw;
	int log_time;
} runtime_opts_t;

typedef struct runtime_dbg_params {
	int fd;
	int lvl;
} runtime_dbg_params_t;

struct runtime;

struct hiddev_event {
        unsigned hid;
        signed int value;
};

typedef struct runtime_ops {
	void (*print_help)(struct runtime *);
	void (*print_version)(struct runtime *);
	void (*print_event)(struct runtime *, void *);
} runtime_ops_t;

void runtime_msg(struct runtime *runtime, int lvl,
	const char *tag, const char *fmt, ...);

typedef struct runtime {
	/* Program executable path (argv[0]) */
	char *exepath;

	/* Program console output tag */
	str_t tag;

	/* Input device file path(/dev/input*) */
	str_t devpath;

	/* Runtime state */
	runtime_state_t state;

	/* Options from commandline */
	runtime_opts_t cmd_opts;

	/* Debug parameters */
	runtime_dbg_params_t dbg;

	/* Input device file descriptor */
	int dev_fd;

	/* Return status */
	status_t status;

	/* Runtime operations */
	runtime_ops_t ops;
} runtime_t;

void runtime_status_set(runtime_t *runtime, status_t s);

const char *runtime_status_str(status_t status);
void runtime_status_print(runtime_t *runtime, status_t status);

void runtime_init(runtime_t *runtime, int argc, char *argv[]);
void runtime_run(runtime_t *runtime);
void runtime_close(runtime_t *runtime);

#endif /* __RUNTIME_H__ */