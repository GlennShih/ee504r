#include "getip.h"
#include <inttypes.h>


u_int32_t get_ip(struct sockaddr *ip)
{
	struct in_addr source_ip_addr;
	
	if (ip->sa_family == AF_INET) {
		struct sockaddr_in *v4 = (struct sockaddr_in *)ip;
		source_ip_addr=v4->sin_addr;
	}
	return ntohl(source_ip_addr.s_addr);
}