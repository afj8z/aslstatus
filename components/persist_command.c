#include "../aslstatus.h"
#include "../lib/util.h"
#include <stdio.h>
#include <string.h>

static void persist_command_cleanup(void *ptr);

void persist_command(char *out, const char *cmd, uint32_t __unused _i,
                     static_data_t *static_data) {
  FILE **fp = (FILE **)static_data->data;

  /* Register cleanup to close pipe on exit */
  if (!static_data->cleanup)
    static_data->cleanup = persist_command_cleanup;

  /* Initialize: popen only once */
  if (!*fp) {
    if (!(*fp = popen(cmd, "r"))) {
      warn("popen('%s')", cmd);
      ERRRET(out);
    }
  }

  /* Read the next line from the persistent script */
  /* This will block until your script outputs a line */
  if (!fgets(out, BUFF_SZ, *fp)) {
    /* If script dies or stream closes, clean up */
    pclose(*fp);
    *fp = NULL;
    ERRRET(out);
  }

  /* Strip trailing newline */
  size_t len = strlen(out);
  if (len > 0 && out[len - 1] == '\n') {
    out[len - 1] = '\0';
  }
}

static void persist_command_cleanup(void *ptr) {
  FILE **fp = (FILE **)ptr;
  if (*fp)
    pclose(*fp);
}
