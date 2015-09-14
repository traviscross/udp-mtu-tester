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

int main(int argc, char **argv) {
  int udp = socket(AF_INET, SOCK_DGRAM, 0);
  int val = IP_PMTUDISC_DONT;
  struct sockaddr_in addr;
  char buf[2048];
  if (argc < 6) {
    fprintf(stderr, "usage: %s ip port startsize maxsize step\n", argv[0]);
    return 1;
  }
  char *ip = argv[1];
  int port = atoi(argv[2]);
  int startsize = atoi(argv[3]);
  int maxsize = atoi(argv[4]);
  int step = atoi(argv[5]);
  memset(&addr, 0, sizeof(addr));
  memset(buf, 0, sizeof(buf));
  addr.sin_family = AF_INET;
  inet_pton(addr.sin_family, ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  setsockopt(udp, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
  int i, r;
  for (i=startsize-28; i <= maxsize-28; i+=step) {
    r=sendto(udp, buf, i, 0, (struct sockaddr*)&addr, sizeof(addr));
    fprintf(stderr, "sent to %s:%d: %d bytes (%d bytes UDP)\n", ip, port, r+28, r);
    struct timespec req, rem;
    req.tv_sec = 0; req.tv_nsec = 20000000;
    while (nanosleep(&req, &rem) == -1 && errno == EINTR)
      memcpy(&req, &rem, sizeof(struct timespec));
  }
  return 0;
}
