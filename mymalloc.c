/*Müslüm Türk
Program that makes its own memory allocator.
It includes implementation of the mymalloc and myfree functions.
*/
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include"mymalloc.h"

Block* bestFitFind();
Block* worstFitFind();
Block* firstFitFind(char* heap_end, int size);

size_t SIZE=1024;
int first_call = 1;

int main(){
    int* a = mymalloc(200);
    int* b = mymalloc(300);
    printHeap();
    myfree(a);
    printHeap();
    myfree(b);
    printHeap();

    return 0;
}

void *mymalloc(size_t size){
    if(size<=0 && size>976){
        printf("The size must be in range 1-976!");
        exit(-1);
    }
    if(first_call==1){
        heap_start=sbrk(SIZE);
        if(heap_start==(void*)-1){
            return NULL;
        }
        heap_start->info.isfree = 1; /*heap_start initializations*/
        heap_start->info.size = SIZE - sizeof(Block) - sizeof(Info);
        heap_start->next = NULL;

        Info* btag = (Info*)((char*)heap_start + sizeof(Block) + heap_start->info.size);
        btag->size = heap_start->info.size;
        btag->isfree = 1;

        free_list = heap_start; /*free_list initializations*/
        free_list->info.isfree = 1;
        free_list->info.size = heap_start->info.size;
        free_list->next = NULL;

        first_call = 0;
    }

    int s = (size + 15)/16;/*16 nın katı yapma*/
    size = 16*s;

    char* heap_end=sbrk(0);/*(char*)heap_start + SIZE*/
    
    switch(strategy){
    case 0: 
        return bestFitFind(heap_end, size);
    case 1:
        return worstFitFind(heap_end, size);
    case 2:
        return firstFitFind(heap_end, size);
    }
}

void *myfree(void *p){
    Block *b = (Block*)((char*)p - sizeof(Block));
    b->info.isfree = 1;
    Info* btag = (Info*)((char*)b + sizeof(Block) + b->info.size);/*btag'in isfree'si 1 yapılır*/
    btag->isfree = 1;

    int isCoalesced = 0;

    //Left Coalecing
    if(!(b<=heap_start)){/*Sol sınırda değilse*/
        Info* prev_btag = (Info*)((char*)b - sizeof(Info));
        if(prev_btag->isfree==1){
            Block* prev = (Block*)((char*)prev_btag - prev_btag->size -sizeof(Block));
            prev->info.size = b->info.size + prev_btag->size + sizeof(Block) + sizeof(Info);/*b soldaki free'nin size'ına eklenir*/
            Info* new_btag = (Info*)((char*)prev + prev->info.size + sizeof(Block));
            new_btag->size = prev->info.size;
            new_btag->isfree = 1;

            b = prev;
            isCoalesced = 1;
        }
    }
    //Right Coalecing
    if(!((b+sizeof(Info)+sizeof(Block)+b->info.size)>=(heap_start + SIZE))){/*b'nin sonu, sağ sınırda değilse*/
        Block* next = (Block*)((char*)b + b->info.size + sizeof(Block) + sizeof(Info)); 
        if(next->info.isfree==1){     
            b->info.size = b->info.size + sizeof(Info) + sizeof(Block) + next->info.size;/*Sağdaki free, b'nin size'ına eklenir*/
            Info* new_btag2 = (Info*)((char*)b + b->info.size + sizeof(Block));
            new_btag2->size = b->info.size;
            new_btag2->isfree = 1;
            next = b;/*b freelistte sağdaki bloğun yerine geçer*/
            b->next = next->next;
            isCoalesced = 1;
        }
    }
    
    if(0 == isCoalesced){ /*Coalesing yapılmadıysa b, buradan free_listin sonuna eklenir*/
        Block* b1 = free_list;
        while(b1 != NULL){
            b1 = (Block*)b1->next;
        }
        b->next = NULL;
        b1 = b;
    }
    
}

Block *split(Block *b, size_t size){
    int bsize = b->info.size;
    Block* b1 = b;
    b1->info.size = size;
    b1->info.isfree = 0;

    Info *btag = (Info*)((char*)b1 + sizeof(Block) + b1->info.size);/*b1 btag*/
    btag->size = b1->info.size;
    btag->isfree = 0;

    Block* b2 = (Block*)((char*)b1 + sizeof(Block) + b1->info.size + sizeof(Info));
    b2->info.size = bsize - b1->info.size - sizeof(Block) - sizeof(Info);
    b2->info.isfree = 1;

    b1->next = (struct Block*)b2;/*b1'in nexti b2'yi gösterir*/
    b=b2;/*free_listte b b2 olur(yani b çıkıp b2 gelir)*/
    b2->next = b->next;/*b2'nin nexti b'nin nextini gösterir*/
    
    Info *btag2 = (Info*)((char*)b2 + sizeof(Block) + b2->info.size);/*b2 btag*/
    btag2->size = b2->info.size;
    btag2->isfree = 1;

    return b1;
}

void printHeap(){
    Block* b = heap_start;
    printf("Blocks\n");
    while(b != NULL){/*Tüm heap list dolaşılır*/
        printf("Free: %d\n", b->info.isfree);
        printf("Size: %d\n", b->info.size);
        printf("----------\n");
        if(((char*)b + sizeof(Block) + b->info.size + sizeof(Info)) < ((char*)heap_start + SIZE)){
            b = (Block*)((char*)b + sizeof(Block) + b->info.size + sizeof(Info));
        }
        else {break;}
    }
}

Block* bestFitFind(char* heap_end, int size){
    int min = 1024;
    Block* minBlock=NULL;

    Block* iter = heap_start;
    while((char*)iter < heap_end){/*size'a uygun en küçük block minBlock'a koyulur*/
        if((iter->info.isfree == 1) && (iter->info.size >= size)){
            if(iter->info.size <= min){
                min = iter->info.size;
                minBlock = iter;
            }
        }
        iter = (Block*)((char *)iter + sizeof(Block) + sizeof(Info) + iter->info.size);
    }

    
    if(minBlock->info.size == size){/*size'lar tam örtüşüyorsa*/
        minBlock->info.isfree = 0;
        Info* btag = (Info*)((char*)minBlock + sizeof(Block) + minBlock->info.size);
        btag->isfree = 0;
        btag->size = minBlock->info.size;

        Block* iter2 = free_list; 
        while((Block*)iter2->next != minBlock && iter2->next != NULL){/*minBlock'u free_listten çıkarmak için döngü*/
            iter2 = (Block*)iter2->next; 
        }
        iter2->next = minBlock->next;

        return (Block*)minBlock->data;
    }
    else if(minBlock->info.size > size){/*size'ın split edilmesi gerekiyorsa*/
        Block* newBlock = split(minBlock, size);
        return (Block*)newBlock->data;
    }
    
    return NULL;
}

Block* worstFitFind(char* heap_end, int size){
    int max = 0;
    Block* maxBlock=NULL;

    Block* iter = heap_start;
    while((char*)iter < heap_end){/*size'a uygun en büyük block maxBlock'a koyulur*/
        if((iter->info.isfree == 1) && (iter->info.size >= size)){
            if(iter->info.size >= max){
                max = iter->info.size;
                maxBlock = iter;
            }
        }
        iter = (Block*)((char *)iter + sizeof(Block) + sizeof(Info) + iter->info.size);
    }
    
    if(maxBlock->info.size == size){/*size'lar tam örtüşüyorsa*/
        maxBlock->info.isfree = 0;
        Info* btag = (Info*)((char*)maxBlock + sizeof(Block) + maxBlock->info.size);
        btag->isfree = 0;
        btag->size = maxBlock->info.size;

        Block* iter2 = free_list; 
        while((Block*)iter2->next != maxBlock && iter2->next != NULL){/*maxBlock'u free_listten çıkarmak için döngü*/
            iter2 = (Block*)iter2->next; 
        }
        iter2->next = maxBlock->next;

           return (Block*)maxBlock->data;
    }
    else if(maxBlock->info.size > size){/*size'ın split edilmesi gerekiyorsa*/
        Block* newBlock = split(maxBlock, size);
        return (Block*)newBlock->data;
    }
    
    return NULL;
}

Block* firstFitFind(char* heap_end, int size){
    Block* b = heap_start;
    while(b->info.isfree != 1 || ((char*)b < heap_end && !(b->info.size >= size))){/*size'a uygun ve free bir block bulunduğunda döngüden çıkar*/
        b = (Block*)((char *)b + sizeof(Block) + sizeof(Info) + b->info.size);
    }
    if((char*)b < heap_end){
        if(b->info.size==size){/*size'lar tam örtüşüyorsa*/
            b->info.isfree = 0;
            Info* btag = (Info*)((char*)b + sizeof(Block) + b->info.size);
            btag->isfree = 0;
            btag->size = b->info.size;

            Block* b1 = free_list; 
            while((Block*)b1->next != b && b1->next != NULL){/*b bloğunu free_listten çıkarmak için döngü*/
                b1 = (Block*)b1->next; 
            }
            b1->next = b->next;

            return (Block*)b->data;
        }
        else if(b->info.size > size){/*size'ın split edilmesi gerekiyorsa*/
            Block* newblock = split(b, size);
            return (Block*)newblock->data;
        }
    }
    return NULL;
}
