#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
  if (argc < 3) {
    fprintf(stderr, "usage: %s ip port\n", argv[0]);
    return 1;
  }
  char *ip = argv[1];
  int port = atoi(argv[2]);
  memset(&addr, 0, sizeof(addr));
  memset(buf, 0, sizeof(buf));
  addr.sin_family = AF_INET;
  inet_pton(addr.sin_family, ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  setsockopt(udp, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
  int i, r;
  for (i=1400-28; i <= 1600-28; i+=200) {
    r=sendto(udp, buf, i, 0, (struct sockaddr*)&addr, sizeof(addr));
    fprintf(stderr, "sendto: %d\n", r);
  }
  return 0;
}
