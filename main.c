#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#define _POSIX_C_SOURCE 200809L

#include "debug.h"
#include "runtime.h"

#define VERSION "0.1"

#define is_terminating(runtime) \
	(runtime->state == RUNTIME_TERMINATING)

#define on_terminating(runtime, label) { \
	if (is_terminating(runtime)) { \
		goto label; \
	} \
}

static inline void print_help_cb(runtime_t *runtime) {
	runtime_msg(runtime, DBG_LVL_INFO, "", "%s: usage: %s [input dev filename]\n",
			runtime->tag, runtime->exepath);
}

static inline void print_version_cb(runtime_t *runtime) {
	runtime_msg(runtime, DBG_LVL_INFO, "", VERSION);
}

static void print_event_cb(runtime_t *runtime, void *ev) {
	str_t code_str, value_str, type_str;
	struct input_event *event = (struct input_event *)ev;

	str_set_int(code_str, 0);
	str_set_int(value_str, 0);
	str_set_int(type_str, 0);

	ev_code_str(event, code_str);
	ev_value_str(event, value_str);
	ev_type_str(event, type_str);

	dbg_info(runtime, ("%s: ( %s {%u}, %s, %s )\n"),
		runtime->tag, type_str, event->type, code_str, value_str);
}

static int ir_ioctl(runtime_t *runtime, unsigned long req, ...) {
	va_list args;
 	va_start(args,req);

	int res = ioctl(runtime->dev_fd, req, args);
	if (res < 0) {
		res = errno;
		dbg_err(runtime, ("ioctl " str(req) "failed, errno: %s (%d)"), strerror(res), res);

		runtime_status_set(runtime, STATUS_ERR_FILE);
	}

	va_end(args);
	return res;
}

int main(int argc, char *argv[]) {
	int desc_size, ret, i;

	struct hidraw_report_descriptor rpt_desc = {0};

	runtime_ops_t ops = {
		.print_help = print_help_cb,
		.print_version = print_version_cb,
		.print_event = print_event_cb,
	};

	runtime_t *runtime = (runtime_t *)malloc(sizeof(*runtime));
	if (!runtime) {
		fprintf(stderr, "[Error]: (malloc) %s", strerror(errno));
		goto exit;
	}

	runtime->dbg.lvl = DBG_LVL_INFO;
	runtime->dbg.fd = STDOUT_FILENO;

	runtime->ops = ops;

	runtime_init(runtime, argc, argv);
	on_terminating(runtime, exit);

	/* Get Report Descriptor Size */
	ir_ioctl(runtime, HIDIOCGRDESCSIZE, &desc_size);
	on_terminating(runtime, exit);

	dbg_info(runtime, "Report Descriptor Size: %d", desc_size);
	
	/* Get HidRaw Report Descriptor */
	rpt_desc.size = desc_size;

	ir_ioctl(runtime, HIDIOCGRDESC, rpt_desc);
	on_terminating(runtime, exit);

	dbg_info(runtime, "Report Descriptor:");

	for (i = 0; i < rpt_desc.size; i++)
		runtime_msg(runtime, DBG_LVL_INFO, "", "%hhx ", rpt_desc.value[i]);

	if (runtime->dbg.fd > STDERR_FILENO)
		runtime_msg(runtime, DBG_LVL_INFO, "", "\n");

	runtime_run(runtime);

exit:
	if (!runtime)
		return STATUS_ERR_NOMEM;

	ret = runtime->status;

	runtime_close(runtime);
	free(runtime);

	return ret;
}