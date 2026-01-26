/*
 * for components info see: wiki/COMPONENTS.md
 *
 * extra configs in can be found in `components_config.h`
 */

/* for usleep */
#define _SEC *1000
#define _MIN *(60 _SEC)
#define _HR *(60 _MIN)

/* interval to run only once */
#define ONCE ((unsigned int)-1)

/* text to show if no value can be retrieved */
static const char unknown_str[] = "n/a";

/* maximum output string length */
#define MAXLEN 256
/*
 * if you want to change buffer size for each segment,
 * then change `BUFF_SZ` in lib/util.h
 */

#define IFC "wlan0" /* wifi interface */

/* clang-format off */
static struct arg_t args[] = {

/* function		format		argument	interval (in ms) */
{ persist_command, "^fg(DBD0C6)| ^fg(A3BE8C)%s", "status_media.sh", 0, END },
{ vpn_status,   "^fg(DBD0C6)| ^fg(E06C75)%s",   "tun0",   5 _SEC,   END },
{ wifi_essid,      "^fg(DBD0C6)Ntk ^fg(A3BE8C)%s",    IFC,          5 _SEC,           END },
{ vol_perc,        "^fg(DBD0C6) | Vol %s",    NULL,          0,           END },
{ battery_perc,    "^fg(DBD0C6) | Bat %s",    "BAT0",        30 _SEC,      END },
{ battery_state, "^fg(DBD0C6)%s",    "BAT0",        30 _SEC,      END },
{ cpu_perc,        "^fg(DBD0C6) | Cpu U=%s",    NULL,          5 _SEC,        END },
{ temp,            "^fg(DBD0C6) T=%sc",                NULL,          5 _SEC,        END },
{ ram_used,        "^fg(DBD0C6) Mem %s",    NULL,          4 _SEC,        END },
{ power_profile, " | Pwm %s", NULL, 0,        END },
{ datetime,        "^fg(DBD0C6) | %s",    "%H:%M",       30 _SEC,      END },
#if USE_X && USE_XKB
// { keymap,		"[%s ",	NULL,		 0,	END },
// { kernel_release, "::[6.16.9-arch1]", NULL, ONCE, END },
#endif

};
/* clang-format on */
