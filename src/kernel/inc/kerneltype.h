#ifndef _KERNELTYPE_H_
#define _KERNELTYPE_H_

#include "interrupt.h"
#include "task.h"
#include "schedule.h"

typedef struct VMA_TO_FILE_FLAGS {
    u64_t upload:1; // 0 不需要加载，需要加载
    u64_t rsv:63;
}vtfflags_t;

typedef struct VMA_TO_FILE{
    u64_t vtf_position;
    u64_t vtf_size;
    file_t* vtf_file;
    vtfflags_t vtf_flag;
}vma_to_file_t;

#endif // _KERNELTYPE_H_