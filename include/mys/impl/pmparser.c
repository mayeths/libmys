/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../debug.h"
#include "../memory.h"
#include "../misc.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//maximum line length in a procmaps file
#define PROCMAPS_LINE_MAX_LENGTH  (4096 + 100)
/**
 * mys_procmap_t
 * @desc hold all the information about an area in the process's  VM
 */
typedef struct mys_procmap_t {
    void* addr_start; 	//< start address of the area
    void* addr_end; 	//< end address
    unsigned long length; //< size of the range

    char perm[5];		//< permissions rwxp
    short is_r;			//< rewrote of perm with short flags
    short is_w;
    short is_x;
    short is_p;

    long offset;	//< offset
    char dev[12];	//< dev major:minor
    int inode;		//< inode of the file that backs the area

    char *pathname;		//< the path of the file that backs the area
    //chained list
    struct mys_procmap_t* next;		//<handler of the chinaed list
} mys_procmap_t;

/**
 * mys_procmaps_t
 * @desc holds iterating information
 */
typedef struct mys_procmaps_t {
    size_t size;
    mys_procmap_t* head;
    mys_procmap_t* current;
} mys_procmaps_t;
/**
 * mys_pmparser_parse
 * @param pid the process id whose memory map to be parser. the current process if pid<0
 * @return an iterator over all the nodes
 */
MYS_PUBLIC mys_procmaps_t* mys_pmparser_parse(int pid);

/**
 * mys_pmparser_next
 * @description move between areas
 * @param p_procmaps_it the iterator to move on step in the chained list
 * @return a procmaps structure filled with information about this VM area
 */
MYS_PUBLIC mys_procmap_t* mys_pmparser_next(mys_procmaps_t* p_procmaps_it);
/**
 * mys_pmparser_free
 * @description should be called at the end to free the resources
 * @param p_procmaps_it the iterator structure returned by mys_pmparser_parse
 */
MYS_PUBLIC void mys_pmparser_free(mys_procmaps_t* p_procmaps_it);

/**
 * _mys_pmparser_split_line
 * @description internal usage
 */
static void _mys_pmparser_split_line(char*buf,char*addr1,char*addr2,char*perm, char* offset, char* device,char*inode,char* pathname);

/**
 * mys_pmparser_print
 * @param map the head of the list
 * @order the order of the area to print, -1 to print everything
 */
MYS_PUBLIC void mys_pmparser_print(mys_procmap_t* map,int order);


MYS_PUBLIC mys_procmaps_t* mys_pmparser_parse(int pid){
    mys_procmaps_t* maps_it = (mys_procmaps_t*)malloc(sizeof(mys_procmaps_t));
    char maps_path[500];
    if(pid>=0 ){
        sprintf(maps_path,"/proc/%d/maps",pid);
    }else{
        sprintf(maps_path,"/proc/self/maps");
    }
    FILE* file=fopen(maps_path,"r");
    if(!file){
        fprintf(stderr,"pmparser : cannot open the memory maps, %s\n",strerror(errno));
        return NULL;
    }
    maps_it->size=0;char buf[PROCMAPS_LINE_MAX_LENGTH];
    mys_procmap_t* list_maps=NULL;
    mys_procmap_t* tmp;
    mys_procmap_t* current_node=list_maps;
    char addr1[20],addr2[20], perm[8], offset[20], dev[10],inode[30],pathname[4096];
    while( !feof(file) ){
        if (fgets(buf,PROCMAPS_LINE_MAX_LENGTH,file) == NULL){
            if(feof(file)) break;
            fprintf(stderr,"pmparser : fgets failed, %s\n",strerror(errno));
            fclose(file);
            return NULL;
        }
        //allocate a node
        tmp=(mys_procmap_t*)malloc(sizeof(mys_procmap_t));
        //fill the node
        _mys_pmparser_split_line(buf,addr1,addr2,perm,offset, dev,inode,pathname);
        //printf("#%s",buf);
        //printf("%s-%s %s %s %s %s\t%s\n",addr1,addr2,perm,offset,dev,inode,pathname);
        //addr_start & addr_end
        sscanf(addr1,"%lx",(long unsigned *)&tmp->addr_start );
        sscanf(addr2,"%lx",(long unsigned *)&tmp->addr_end );
        //size
        tmp->length=(unsigned long)((unsigned long)tmp->addr_end-(unsigned long)tmp->addr_start);
        //perm
        strcpy(tmp->perm,perm);
        tmp->is_r=(perm[0]=='r');
        tmp->is_w=(perm[1]=='w');
        tmp->is_x=(perm[2]=='x');
        tmp->is_p=(perm[3]=='p');

        //offset
        sscanf(offset,"%lx",&tmp->offset );
        //device
        strcpy(tmp->dev,dev);
        //inode
        tmp->inode=atoi(inode);
        //pathname
        tmp->pathname = strdup(pathname);
        tmp->next=NULL;
        //attach the node
        if(maps_it->size==0){
            list_maps=tmp;
            list_maps->next=NULL;
            current_node=list_maps;
        }
        current_node->next=tmp;
        current_node=tmp;
        maps_it->size++;
        //printf("%s",buf);
    }

    //close file
    fclose(file);


    //g_last_head=list_maps;
    maps_it->head = list_maps;
    maps_it->current =  list_maps;
    return maps_it;
}


MYS_PUBLIC mys_procmap_t* mys_pmparser_next(mys_procmaps_t* p_procmaps_it){
    if(p_procmaps_it->current == NULL)
        return NULL;
    mys_procmap_t* p_current = p_procmaps_it->current;
    p_procmaps_it->current = p_procmaps_it->current->next;
    return p_current;
}



MYS_PUBLIC void mys_pmparser_free(mys_procmaps_t* p_procmaps_it){
    mys_procmap_t* maps_list = p_procmaps_it->head;
    if(maps_list==NULL) return ;
    mys_procmap_t* act=maps_list;
    mys_procmap_t* nxt=act->next;
    while(act!=NULL){
        free(act->pathname);
        free(act);
        act=nxt;
        if(nxt!=NULL)
            nxt=nxt->next;
    }
    free(p_procmaps_it);
}


static void _mys_pmparser_split_line(
        char*buf,char*addr1,char*addr2,
        char*perm,char* offset,char* device,char*inode,
        char* pathname){
    //
    int orig=0;
    int i=0;
    //addr1
    while(buf[i]!='-'){
        addr1[i-orig]=buf[i];
        i++;
    }
    addr1[i]='\0';
    i++;
    //addr2
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' '){
        addr2[i-orig]=buf[i];
        i++;
    }
    addr2[i-orig]='\0';

    //perm
    while(buf[i]=='\t' || buf[i]==' ')
        i++;
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' '){
        perm[i-orig]=buf[i];
        i++;
    }
    perm[i-orig]='\0';
    //offset
    while(buf[i]=='\t' || buf[i]==' ')
        i++;
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' '){
        offset[i-orig]=buf[i];
        i++;
    }
    offset[i-orig]='\0';
    //dev
    while(buf[i]=='\t' || buf[i]==' ')
        i++;
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' '){
        device[i-orig]=buf[i];
        i++;
    }
    device[i-orig]='\0';
    //inode
    while(buf[i]=='\t' || buf[i]==' ')
        i++;
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' '){
        inode[i-orig]=buf[i];
        i++;
    }
    inode[i-orig]='\0';
    //pathname
    pathname[0]='\0';
    while(buf[i]=='\t' || buf[i]==' ')
        i++;
    orig=i;
    while(buf[i]!='\t' && buf[i]!=' ' && buf[i]!='\n'){
        pathname[i-orig]=buf[i];
        i++;
    }
    pathname[i-orig]='\0';

}

MYS_PUBLIC void mys_pmparser_print(mys_procmap_t* map, int order){

    mys_procmap_t* tmp=map;
    int id=0;
    if(order<0) order=-1;
    while(tmp!=NULL){
        //(unsigned long) tmp->addr_start;
        if(order==id || order==-1){
            printf("Backed by:\t%s\n",strlen(tmp->pathname)==0?"[anonym*]":tmp->pathname);
            printf("Range:\t\t%p-%p\n",tmp->addr_start,tmp->addr_end);
            printf("Length:\t\t%ld\n",tmp->length);
            printf("Offset:\t\t%ld\n",tmp->offset);
            printf("Permissions:\t%s\n",tmp->perm);
            printf("Inode:\t\t%d\n",tmp->inode);
            printf("Device:\t\t%s\n",tmp->dev);
        }
        if(order!=-1 && id>order)
            tmp=NULL;
        else if(order==-1){
            printf("#################################\n");
            tmp=tmp->next;
        }else tmp=tmp->next;

        id++;
    }
}
