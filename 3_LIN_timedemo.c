/* Trivial libtrace program that prints a count of the number of packets
 * observed every 10 seconds in a trace.
 * Designed to demonstrate the use of trace_get_timeval()
 */

#include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <inttypes.h>
#include <assert.h>
#include <getopt.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libtrace.h"

# define HEAD 701
uint32_t next_report = 0;
uint64_t count = 0;
struct timeval ts;


typedef struct node {
	uint32_t val;
	int count;
	struct node *next;
}list_node;

typedef list_node *link;
link head[HEAD], cur, pre;


void initializa (){
	int i;
	for(i=0; i<HEAD; i++){
		head[i] = (link)malloc(sizeof(list_node));
		head[i]->next = NULL; 
	}	
}

int Hash_index (unsigned long int h){
	return h % HEAD;
}

void add_node (unsigned long int fkey)
{
    link newnode, tmp;
	int addr;
	newnode = (link)malloc(sizeof(list_node));
	addr = Hash_index(fkey);
	newnode -> val = fkey;
	tmp = head[addr];
	pre = tmp;
	cur = tmp -> next;
    	
	while(cur != NULL)
	{
		if(cur -> val == fkey)
		{
			cur -> count++;
			break;
		}
		pre = cur;
		cur = cur ->next;
	}
	
	if(cur == NULL)
	{
		newnode -> next = cur;
		pre -> next = newnode;
	}

}

void print_dist(){
	link tmp;
	int i, distin_num = 0;

   		for(i=0; i<HEAD; i++){
		    tmp = head[i];
            cur = tmp -> next;
		    while(cur != NULL){
		        distin_num++;
                printf("Data = %u , counter = %u\n", cur-> val ,(cur -> count)+1);
		        cur	= cur -> next;
            }
            tmp -> next = NULL;
        }
    printf("distinct = %d\n" ,distin_num);
    distin_num = 0;
    }


uint32_t get_ip(struct sockaddr *ip) {

	char str[20];
	struct in_addr source_ip_addr;//yk
    uint32_t c;
    int distin_num = 0;
    
	
	/* Check the sockaddr family so we can cast it to the appropriate
	 * address type, IPv4 or IPv6 */
	if (ip->sa_family == AF_INET) {
		/* IPv4 - cast the generic sockaddr to a sockaddr_in */
		struct sockaddr_in *v4 = (struct sockaddr_in *)ip;
		//printf("src_IP = %s \n", inet_ntop(AF_INET, &(v4->sin_addr), str, 20));
		source_ip_addr=v4->sin_addr;
    	c = ntohl(source_ip_addr.s_addr);
        //printf("IP val = %d\n\n", c);
        return c;
	}   

}

void per_packet(libtrace_packet_t *packet)
{
		
	struct sockaddr_storage addr;
	struct sockaddr *addr_ptr;
    struct timeval ts;
	
	
	ts = trace_get_timeval(packet);
	addr_ptr = trace_get_source_address(packet, (struct sockaddr *)&addr);  

	/* No IP address? Print "NULL" instead */
	if (addr_ptr == NULL){
		printf("NULL ");
    }
	else {
        add_node(get_ip(addr_ptr));
    }
    
	if (next_report == 0) {
		next_report = ts.tv_sec + 10;
		printf("Time\t\tPackets\n");
	}

	
	while (ts.tv_sec >= next_report) {
		print_dist();
        next_report += 10;
        printf("inter_packet count = %ld\n", count);
        printf("---------------------------------\n");
        count = 0;
	}
    count += 1;
    

}


/* Due to the amount of error checking required in our main function, it
 * is a lot simpler and tidier to place all the calls to various libtrace
 * destroy functions into a separate function.
 */
void libtrace_cleanup(libtrace_t *trace, libtrace_packet_t *packet) {
	
	if (trace)
		trace_destroy(trace);

	if (packet)
		trace_destroy_packet(packet);

}

int main(int argc, char *argv[])
{
	/* This is essentially the same main function from readdemo.c */
	
	libtrace_t *trace = NULL;
	libtrace_packet_t *packet = NULL;
    
	
	int i = 0;
	//unsigned long int fkey;
	
	/* Ensure we have at least one argument after the program name */
        if (argc < 2) {
                fprintf(stderr, "Usage: %s inputURI\n", argv[0]);
                return 1;
        }	

	packet = trace_create_packet();
       

	if (packet == NULL) {
		perror("Creating libtrace packet");
		libtrace_cleanup(trace, packet);
		return 1;
	}

	trace = trace_create(argv[1]);

	if (trace_is_err(trace)) {
		trace_perror(trace,"Opening trace file");
		libtrace_cleanup(trace, packet);
		return 1;
	}

	if (trace_start(trace) == -1) {
		trace_perror(trace,"Starting trace");
		libtrace_cleanup(trace, packet);
		return 1;
	}

	/*add by max*/    
	initializa ();
	while (trace_read_packet(trace,packet)>0) {
        per_packet(packet);
	}
    print_dist();

	if (trace_is_err(trace)) {
		trace_perror(trace,"Reading packets");
		libtrace_cleanup(trace, packet);
		return 1;
	}


	libtrace_cleanup(trace, packet);
	return 0;
}
