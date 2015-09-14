#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

static const int udp_hlen = 28;
static const int maxbuf = 65535;

int err(char *m) {
  fprintf(stderr, "error: %s\n", m);
  return 1;
}

int pmtudisc_val(char *s) {
  if (!strcmp(s,"want")) return IP_PMTUDISC_WANT;
  if (!strcmp(s,"dont")) return IP_PMTUDISC_DONT;
  if (!strcmp(s,"do")) return IP_PMTUDISC_DO;
  if (!strcmp(s,"probe")) return IP_PMTUDISC_PROBE;
  return -1;
}

int main(int argc, char **argv) {
  int udp = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in addr;
  char buf[maxbuf];
  if (argc < 8) {
    fprintf(stderr, "usage: %s ip port want|dont|do|probe startsize maxsize step delay_ms\n", argv[0]);
    return 1;
  }
  char *ip = argv[1];
  int port = atoi(argv[2]);
  int pmtudisc_arg = pmtudisc_val(argv[3]);
  int startsize = atoi(argv[4]);
  int maxsize = atoi(argv[5]);
  int step = atoi(argv[6]);
  int delay_ms = atoi(argv[7]);
  if (port < 0 || port > 65535) return err("invalid port");
  if (pmtudisc_arg < 0) return err("invalid pmtudisc type (want|dont|do|probe)");
  if (startsize < udp_hlen) return err("startsize must be at least 28 bytes (UDP header length)");
  if (maxsize > maxbuf) return err("maxsize must be less than 2^16-1");
  if (startsize > maxsize) return err("startsize must be less than maxsize");
  if (step < 0) return err("step must be >=0");
  if (delay_ms < 0) return err("delay_ms must be >=0");
  memset(&addr, 0, sizeof(addr));
  memset(buf, 0, sizeof(buf));
  addr.sin_family = AF_INET;
  inet_pton(addr.sin_family, ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  setsockopt(udp, IPPROTO_IP, IP_MTU_DISCOVER, &pmtudisc_arg, sizeof(int));
  int i, r;
  for (i=startsize-udp_hlen; i <= maxsize-udp_hlen; i+=step) {
    r=sendto(udp, buf, i, 0, (struct sockaddr*)&addr, sizeof(addr));
    fprintf(stderr, "sent to %s:%d: %d bytes (%d bytes UDP)\n", ip, port, r+udp_hlen, r);
    struct timespec req, rem;
    req.tv_sec = delay_ms / 1000;
    req.tv_nsec = (delay_ms % 1000) * 1000000;
    while (nanosleep(&req, &rem) == -1 && errno == EINTR)
      memcpy(&req, &rem, sizeof(struct timespec));
  }
  return 0;
}
