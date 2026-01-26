#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aslstatus.h"
#include "../lib/util.h"

static void power_cleanup(void *ptr);

/*
 * Maps known TLP strings to abbreviations.
 * Scans the output of 'tlp-stat -m' for these keywords.
 */
static const char *
resolve_tlp_status(const char *raw_output)
{
	if (strstr(raw_output, "performance")) return "PRF";
	if (strstr(raw_output, "balanced"))    return "BAL";
	if (strstr(raw_output, "power-saver")) return "SAV";
	if (strstr(raw_output, "AC"))          return "AC";
	if (strstr(raw_output, "battery"))     return "BAT";
	if (strstr(raw_output, "BAT"))         return "BAT";
	
	/* Fallback: try to return a clean part of the string or "unk" */
	return "UNK";
}

/* * Helper to run tlp-stat -m and write the result to 'out'.
 */
static void
update_status(char *out)
{
	FILE *fp;
	char  buf[256];
	size_t len;

	/* * tlp-stat -m prints the active profile (e.g., "balanced") 
	 * or mode (e.g., "Operation Mode: AC").
	 */
	if ((fp = popen("tlp-stat -m", "r"))) {
		if (fgets(buf, sizeof(buf), fp)) {
			/* Strip newline for cleaner debugging/fallback */
			buf[strcspn(buf, "\n")] = '\0';
			bprintf(out, "%s", resolve_tlp_status(buf));
		}
		pclose(fp);
	}
}

void
power_profile(char           *out,
              const char __unused *_a,
              uint32_t __unused    _i,
              static_data_t       *static_data)
{
	FILE **fp = (FILE **)static_data->data;
	char   buf[BUFSIZ];

	/* Initialize on first run */
	if (*fp == NULL) {
		/* Get initial state immediately */
		update_status(out);

		/* * Start monitoring for power supply changes.
		 * TLP is triggered by udev rules on the 'power_supply' subsystem.
		 * We monitor these events to know when to re-query TLP.
		 */
		*fp = popen("udevadm monitor --udev --subsystem-match=power_supply", "r");

		if (!*fp) {
			warn("popen 'udevadm monitor'");
			ERRRET(out);
		}

		if (!static_data->cleanup)
			static_data->cleanup = power_cleanup;

		return;
	}

	/* Subsequent runs: Block and wait for an event from udevadm */
	while (fgets(buf, sizeof(buf), *fp)) {
		/* * We filter for "UDEV" events because TLP runs as a udev rule.
		 * Waiting for the UDEV event (post-processing) ensures TLP 
		 * has likely been triggered.
		 */
		if (strstr(buf, "UDEV")) {
			update_status(out);
			return; /* Return to update the status bar */
		}
	}

	/* If pipe closes/dies, clean up */
	pclose(*fp);
	*fp = NULL;
	ERRRET(out);
}

static void
power_cleanup(void *ptr)
{
	FILE **fp = (FILE **)ptr;
	if (*fp)
		pclose(*fp);
}
