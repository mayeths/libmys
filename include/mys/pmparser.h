/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "_config.h"
#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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

MYS_PUBLIC void mys_pmparser_init();
MYS_PUBLIC mys_procmaps_t *mys_pmparser_self();

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
 * mys_pmparser_print
 * @param map the head of the list
 * @order the order of the area to print, -1 to print everything
 */
MYS_PUBLIC void mys_pmparser_print(mys_procmap_t* map,int order);
