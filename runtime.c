#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "runtime.h"
#include "debug.h"

static const char *_status_msgs[] = {
	INIT_ELEM(STATUS_OK, "Success"),
	INIT_ELEM(STATUS_ERR_NOMEM, "Not enough memory"),
	INIT_ELEM(STATUS_ERR_SHELLPARAMS, "Wrong command line arguments"),
	INIT_ELEM(STATUS_ERR_FILE, "File error"),
	INIT_ELEM(STATUS_ERR_UNKNOWN, "Unknown error")
};

const char *runtime_status_str(unsigned int code) {
	return ((code > STATUS_ERR_UNKNOWN)?
		_status_msgs[STATUS_ERR_UNKNOWN] : _status_msgs[code]);
}

static void runtime_terminate(runtime_t *runtime, status_t status) {
	runtime->state  = RUNTIME_TERMINATING;
	runtime->status = status;
}

void runtime_msg(struct runtime *runtime, int lvl,
		const char *tag, const char *fmt, ...) {
	str_t buf;
	va_list args;

	if (runtime->dbg.lvl <= lvl) {
		va_start(args,fmt);

		str_set_str(buf, (char *)tag);
		snprintf(buf + str_len(tag), MAX_STR_LEN - str_len(tag), fmt, args);

		va_end(args);

		write(runtime->dbg.fd, buf, MIN(str_len(buf), MAX_STR_LEN));
	}
}

void runtime_status_set(runtime_t *runtime, status_t s) {
	if (!runtime)
		return;

	if (s != STATUS_OK)
		return runtime_terminate(runtime, s);

	runtime->status = STATUS_OK;
}

static void runtime_parse_shell_params(runtime_t *runtime, int argc, char *argv[]) {
	int idx = 2;

	memset(&runtime->cmd_opts, 0, sizeof(runtime_opts_t));

	while (idx < argc) {
		if (str_equal(argv[idx], "--help")) {
			runtime->ops.print_help(runtime);
			runtime_terminate(runtime, STATUS_OK);

			break;
		}
		else if (str_equal(argv[idx], "--debug")) {
			runtime->cmd_opts.debug = 1;
			runtime->dbg.lvl = DBG_LVL_DEBUG;
		}
		else if (str_equal(argv[idx], "--quiet")) {
			runtime->cmd_opts.quiet = 1;
			runtime->cmd_opts.debug = 0;
			runtime->dbg.lvl = DBG_LVL_NONE;
		}
		else if (str_equal(argv[idx], "--log-time")) {
			runtime->cmd_opts.log_time = 1;
		}
		else if (str_equal(argv[idx], "--version")) {
			runtime->ops.print_version(runtime);
			runtime_terminate(runtime, STATUS_OK);

			break;	
		}
		else if (str_equal(argv[idx], "--force")) {

		}
		else {
			// if (argv[idx])
			// 	runtime_status_set(runtime, STATUS_ERR_SHELLPARAMS);

			break;
		}

		idx++;
	}
}

void runtime_init(runtime_t *runtime, int argc, char *argv[]) {
	str_set_str(runtime->tag, TAG_NAME);

	runtime->state = RUNTIME_UNINITIALIZED;

	if (argc < 2) {
		runtime->ops.print_help(runtime);
		runtime_status_set(runtime, STATUS_ERR_SHELLPARAMS);

		return;
	}

	runtime_parse_shell_params(runtime, argc, argv);
	if (runtime->state == RUNTIME_TERMINATING)
		return;

	runtime->dev_fd = open(argv[1], O_RDONLY);
	if (runtime->dev_fd < 0) {
		runtime_status_set(runtime, STATUS_ERR_FILE);
		dbg_err(runtime, "dev_fd: %d, argv[1]: %s\n", runtime->dev_fd, argv[1]);

		return;
	}

	runtime->exepath = argv[1];
	runtime->state = RUNTIME_OPENED;	
}

void runtime_close(runtime_t *runtime) {
	if (!runtime)
		return;

	close(runtime->dev_fd);
	close(runtime->dbg.fd);
}

void runtime_run(runtime_t *runtime) {
	struct input_event event = {0};
	unsigned long counter = 0;

	int bytes;

	while (1) {
		memset(&event, 0, sizeof(event));

		counter++;
		bytes = read(runtime->dev_fd, &event, sizeof(event));

		if (bytes < 0) {
			runtime_status_set(runtime, STATUS_ERR_FILE);

			dbg_err(runtime, "read() error: %s\n", strerror(errno));
			dbg_err(runtime, "bytes: %d, iter: %lu\n", bytes, counter);
			return;
		}

		if (!bytes) {
			runtime_terminate(runtime, STATUS_OK);

			dbg_err(runtime, "Reached the end of data, terminating.");
			return;
		}

		if (bytes != sizeof(event)) {
			dbg_err(runtime, ("read() Invalid event size (%d \\ %lu)\n"),
				bytes, sizeof(event));

			return;
		}
		
		runtime->ops.print_event(runtime, &event);
	}
}