#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#define init_hash_size 131072

typedef struct list
{
    uint64_t number;
    uint64_t name;
    struct list *next;
}List;
List *head[init_hash_size], *current, *previous;
void init_List();
void traverse(int index, FILE *file_out);
int count_List(int index);

int main(int argc, char **argv)
{
    FILE *file_in, *file_out;

    if (argc!=4) 
    {
        fprintf(stderr, "Usage: %s <trace file> <output file name> <hash_size>\n", argv[0]);
        exit(0);
    }
    if((file_in=fopen(argv[1],"r"))==NULL)
    {
        printf("Error : Can't open pattern file \"%s\"\n",argv[1]);
        exit(0);
    }
    //************************************************************************//
    char *output_file=argv[2];
    file_out = fopen(output_file, "wb");
    uint32_t hash_size = atol(argv[3]);
    uint64_t n; 
    int i, index; 
    init_List();
    while(fscanf(file_in, "%lu", &n)==1)
    {
        index=n%hash_size;
        previous = head[index];
        current = head[index]->next;
        while(current!=NULL)
        {
            if(current->name==n)
            {
                current->number++;
                break;
            }
            previous = current;
            current = current->next;
        }
        if(current==NULL)
        {
            List *ptr;//欲插入的資料
            ptr = (List*)malloc(sizeof(List));
            ptr->number = 1;
            ptr->name = n;
            ptr->next = current;
            previous->next = ptr;
        }
        
    }
    fclose(file_in);

    int sum=0,distinct_items;
    for(i=0;i<hash_size;i++)
    {
        distinct_items=count_List(i);
        sum+=distinct_items;
        fprintf(file_out,"--------table %d-------- Distinct Items:%d\n",i,distinct_items);
        traverse(i,file_out);
    }
    fprintf(file_out,"Total Distinct items : %d\n",sum);
    fclose(file_out);
    printf("Program Completion.\n");
    return 0;
}

void init_List()
{
    int i;
    for(i=0;i<init_hash_size;i++)
    {
        head[i] = (List*)malloc(sizeof(List));
        head[i]->next = NULL;
    }
}
void traverse(int index,FILE *file_out)
{
    if(head[index]->next == NULL) fprintf(file_out,"List is NULL\n");
    else
    {
        current = head[index]->next;
        fprintf(file_out, "\n");
        while(current != NULL)
        {
            fprintf(file_out, " %11lu   %lu\n",current->name,current->number);
            //printf("%lu %lu\n",current->number,current->name);
            current = current->next;
        }
        fprintf(file_out, "\n");
    }
}

int count_List(int index)
{
    int length=0;
    current = head[index]->next;
    while(current!=NULL)
    {
        current = current->next;
        length++;
    }
    return length;
}