#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include "../aslstatus.h"
#include "../lib/util.h"

/* * Zero-width space.
 * Used to trick aslstatus into thinking it has content (skipping "n/a"),
 * but rendering nothing
 */
#define INVISIBLE_CHAR "\xE2\x80\x8B"

void vpn_status(char *out, const char *interface, uint32_t __unused _i,
                static_data_t __unused *_p) {
  struct ifaddrs *ifaddr, *ifa;
  char host[NI_MAXHOST];
  int s;
  int found = 0;

  if (getifaddrs(&ifaddr) == -1) {
    /* Error: fallback to invisible */
    bprintf(out, "%s", INVISIBLE_CHAR);
    return;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    /* Check for IPv4 (AF_INET) match on the interface */
    if (strcmp(ifa->ifa_name, interface) == 0 &&
        ifa->ifa_addr->sa_family == AF_INET) {

      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                      NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      if (s == 0) {
        /* Found it! Write the full label + IP here */
        bprintf(out, "Vpn %s ", host);
        found = 1;
      }
      break;
    }
  }

  freeifaddrs(ifaddr);

  if (!found) {
    /* VPN off: Write invisible char so 'n/a' doesn't appear */
    bprintf(out, "%s", INVISIBLE_CHAR);
  }
}
