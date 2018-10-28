/* Trivial libtrace program that prints a count of the number of packets
 * observed every 10 seconds in a trace.
 * Designed to demonstrate the use of trace_get_timeval()
 */
#include "libtrace.h"
#include "getip.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define HASH_TABLE_SIZE 100
#define timeinterval 10

uint64_t intervalcount = 0;
uint64_t TotalPacketCount = 0;
uint32_t next_report = 0;
uint64_t distinct_num = 0;
uint32_t interval_number = 1;
uint32_t packet_usec;

/*  defined the struct of linked list node  */
typedef struct node{
    uint32_t data;
    uint64_t counter;
    struct node *next;
}list_node;

typedef list_node *list_pointer;

list_pointer head[HASH_TABLE_SIZE], current, previous;

/* initialize the linked list array  */
void initialize(){
    int i;
    for(i=0;i<HASH_TABLE_SIZE;i++){
        head[i] = (list_pointer)malloc(sizeof(list_node));
        head[i]->next = NULL;
    }
}

/*  Hash function by modulo  */
int lazyhash(unsigned long key){
    return key % HASH_TABLE_SIZE;
}

/*  insert the node into linked list  */
void insert(unsigned long value){
    int address;
    list_pointer ptr, tmp;

    address = lazyhash(value);
    tmp = head[address];

    //initial previous and current pointer position
    previous = tmp;
    current = tmp -> next;
    //the next node is not the end of list(NOT NULL)
    while(current != NULL)
	{
        //if exist, add counter by 1, then break while loop
        if(current->data == value)
		{
            current->counter++;
            break;
        }
        //just keep looking forward
        previous = current;
        current = current -> next;
    }
    //only add new node when it goes to the end of list(and didn't find any exist same data/value)
    if(current == NULL)
	{
	    /* create a new node ptr */
		ptr = (list_pointer)malloc(sizeof(list_node));
    	/* Save the new value to data in struct*/
		ptr -> data = value;
		/* initialize the counter to 1 */	
		ptr -> counter = 1;
		ptr->next = current;
        previous->next = ptr;
    }
}



void print_linkedlist(){
	int i;
	list_pointer tmp;
		
	for(i=0;i<HASH_TABLE_SIZE;i++){

		tmp = head[i];
		current = tmp->next;

		while(current != NULL){
			distinct_num++;
			printf("%u \t counter = %lu\n",current->data, (current->counter));
			current = current->next;
			//current->counter = 0;
		}
		/* Reset linked list*/
		tmp->next = NULL;
	}
}


/*
*
*  Start point of timedemo part, counted by 10 second
*
*/
static void per_packet(libtrace_packet_t *packet)
{
	struct sockaddr_storage addr;
	struct sockaddr *addr_ptr;	
	struct timeval ts;
	
	/* Get the timestamp for the current packet */
	ts = trace_get_timeval(packet);
	/* Get the source IP address for the current packet*/
	addr_ptr = trace_get_source_address(packet, (struct sockaddr *)&addr);
	insert(get_ip(addr_ptr));

	//printf("%lu\n",ts.tv_sec);
	/* If next_report is zero, then this is the first packet from the
	 * trace so we need to determine the time at which the first report
	 * must occur, i.e. 10 seconds from now. */
	if (next_report == 0) {
		next_report = ts.tv_sec + timeinterval;
		packet_usec = ts.tv_usec;
        /*  for counting time in u seconds  */
        //next_report = ts.tv_usec + 10;
		
        /* This is also a good opportunity to print column headings */
		
		printf("Time Start\n");
		printf("------------------------------------------\n");
		printf("Interval %u \n", interval_number);
		printf("SIP\t\tDistinctPacketCount\n");
	}

	/* Check whether we need to report a packet count or not.
	 *
	 * If the timestamp for the current packet is beyond the time when the
	 * next report was due then we have to output our current count and
	 * reset it to zero.
	 *
	 * Note that I use a while loop here to ensure that we correctly deal
	 * with periods in which no packets are observed.
	 */
	while ((uint32_t)ts.tv_sec >= next_report && (uint32_t)ts.tv_usec > packet_usec) {
	//while ((uint32_t)ts.tv_sec >= next_report) {
  
		print_linkedlist();

		interval_number++;
		/* Print a timestamp for the report and the packet count */
		printf("distinct_num = %" PRIu64 "\n", distinct_num);
		printf("time=%u \tIntervalPacketCount=%" PRIu64 "\n", next_report, intervalcount);
        printf("------------------------------------------\n");
		printf("Interval %u \n", interval_number);
		printf("SIP\t\tDistinctPacketCount\n");
	      
		/* Reset the counter */
		distinct_num = 0;
		intervalcount = 0;
		/* Determine when the next report is due */
		next_report += timeinterval;
	}

	/* No matter what else happens during this function call, we still
	 * need to increment our counter */	
	intervalcount += 1;
	TotalPacketCount += 1;
}

/* Due to the amount of error checking required in our main function, it
 * is a lot simpler and tidier to place all the calls to various libtrace
 * destroy functions into a separate function.
 */
static void libtrace_cleanup(libtrace_t *trace, libtrace_packet_t *packet) {
	
	/* It's very important to ensure that we aren't trying to destroy
         * a NULL structure, so each of the destroy calls will only occur
         * if the structure exists */
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

	/* initialize the linked list array  */
	initialize();

	while (trace_read_packet(trace,packet)>0) {
		per_packet(packet);
	}

	print_linkedlist();

	if (trace_is_err(trace)) {
		trace_perror(trace,"Reading packets");
		libtrace_cleanup(trace, packet);
		return 1;
	}

	printf("distinct_num = %" PRIu64 "\n", distinct_num);
	printf("time=%u \tintervalcount=%" PRIu64 "\n", next_report, intervalcount);
	printf("time=%u \tTotalPacketCount=%" PRIu64 "\n", next_report, TotalPacketCount);

	libtrace_cleanup(trace, packet);
	return 0;
}