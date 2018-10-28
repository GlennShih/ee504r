#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#define HASH_TABLE_SIZE 1000


typedef struct node{
    unsigned long data;
    int counter;
    struct node *next;
}list_node;

typedef list_node *list_pointer;

list_pointer head[HASH_TABLE_SIZE], current, previous;

void initialize(){
    int i;
    for(i=0;i<HASH_TABLE_SIZE;i++){
        head[i] = (list_pointer)malloc(sizeof(list_node));
        head[i]->next = NULL;
    }
}

int lazyhash(unsigned long key){
    return key % HASH_TABLE_SIZE;
}

void insert(unsigned long value){
    int address;
    list_pointer ptr, tmp;
    ptr = (list_pointer)malloc(sizeof(list_node));
    ptr -> data = value;
    address = lazyhash(value);
    tmp = head[address];
    //printf("lazyhash=%d",tmp);
    //initial previous and current pointer position
    previous = tmp;
    current = tmp -> next;
    //the next node is not the end of list(NOT NULL)
    while(current != NULL){
        //if exist, add counter by 1, then break while loop
        if(current->data == value){
            current->counter++;
            break;
        }
        //just keep looking forward
        previous = current;
        current = current -> next;
    }
    //only add new node when it goes to the end of list(and didn't find any exist same data/value)
    if(current == NULL){
        ptr->next = current;
        previous->next = ptr;
    }
}

void traverse(){
    int i;
    list_pointer tmp;
    for(i=0;i<HASH_TABLE_SIZE;i++){
        tmp = head[i];
        if(tmp->next == NULL){
            printf("empty linked list, can't print");
        }
        else{
            current = tmp->next;
            while(current != NULL){
                printf("data = %ld, counter = %d\n",current->data, (current->counter)+1);
                current = current->next;
            }
        }
    }


}
/*
int count_length(){
    int length = 0;
    current = head -> next;
    while(current != NULL){
        length++;
        current = current -> next;
    }
    return length;
}

int count_total(){
    int total = 0, i;
    current = head -> next;
    while(current != NULL){
        total = (current->counter + 1 ) + total;
        current = current -> next;
    }
    return total;
}
*/
int main(int argc, char **argv){
    unsigned long key;
    int i,distinct_num=0;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s  <input .txt file> <output .txt file>\n", argv[0]);
        return 1;
        }
    
    FILE *file_in=NULL;
    FILE *file_out;
    char *output_name=argv[2];
	file_in = fopen(argv[1],"r");
    file_out = fopen(output_name, "wb");

    
    initialize();
	while(fscanf(file_in, "%lu", &key) != EOF){
        //printf("%lu\n", key);
        insert(key);
    }
   
    /* 
     *  Print to file
    */
    list_pointer tmp;
    for(i=0;i<HASH_TABLE_SIZE;i++){
        tmp = head[i];
        if(tmp->next == NULL){
            //printf("empty linked list, can't print");
        }
        else{
            current = tmp->next;
            while(current != NULL){
                distinct_num++;
                fprintf(file_out ,"%ld, counter = %d\n",current->data, (current->counter)+1);
                current = current->next;
            }
        }
    }
    fprintf(file_out ,"distinct = %d",distinct_num);

    /*close .txt file*/   
    fclose(file_in);
    fclose(file_out);
}