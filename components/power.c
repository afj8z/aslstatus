#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aslstatus.h"
#include "../lib/util.h"

static void power_cleanup(void *ptr);

static const char *get_abbreviation(const char *profile) {
  if (!strcmp(profile, "performance"))
    return "PRF";
  if (!strcmp(profile, "balanced"))
    return "BAL";
  if (!strcmp(profile, "power-saver"))
    return "SAV";
  return profile; /* Fallback to original if unknown */
}

void power_profile(char *out, const char __unused *_a, uint32_t __unused _i,
                   static_data_t *static_data) {
  FILE **fp = (FILE **)static_data->data;
  char buf[BUFSIZ];
  char *start, *end;
  int parsing_value = 0;

  /* Initialize on first run */
  if (*fp == NULL) {
    FILE *init_fp = popen("powerprofilesctl get", "r");
    if (init_fp) {
      if (fgets(buf, sizeof(buf), init_fp)) {
        buf[strcspn(buf, "\n")] = 0;
        bprintf(out, "%s", get_abbreviation(buf));
      }
      pclose(init_fp);
    }

    /* Start monitoring for future changes */
    *fp = popen("dbus-monitor --system "
                "\"type='signal',sender='net.hadess.PowerProfiles',interface='"
                "org.freedesktop.DBus.Properties',member='PropertiesChanged'\"",
                "r");

    if (!*fp) {
      warn("popen 'dbus-monitor'");
      ERRRET(out);
    }

    if (!static_data->cleanup)
      static_data->cleanup = power_cleanup;

    return;
  }

  /* Subsequent runs: Block and wait for an event from dbus-monitor */
  while (fgets(buf, sizeof(buf), *fp)) {
    if (strstr(buf, "\"ActiveProfile\"")) {
      parsing_value = 1;
      continue;
    }

    /* If we are looking for the value, find the variant string */
    if (parsing_value && (start = strstr(buf, "string \""))) {
      start += 8;
      end = strchr(start, '"');
      if (end) {
        *end = '\0';
        bprintf(out, "%s", get_abbreviation(start));
        return;
      }
    }
  }

  /* If dbus-monitor dies, close and reset */
  pclose(*fp);
  *fp = NULL;
  ERRRET(out);
}

static void power_cleanup(void *ptr) {
  FILE **fp = (FILE **)ptr;
  if (*fp)
    pclose(*fp);
}
