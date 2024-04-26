// open_flag参数如下，定义了一系列文件操作标志位宏
#define O_RDONLY 00000000  /*Open read-only 00*/
#define O_WRONLY 00000001  /*Open write-only 01*/
#define O_RDWR 00000002    /*Open read/write 10*/
#define O_ACCMODE 00000003 /*Mask for file access modes 11*/

#define O_CREAT 00000100  /*Create file if it does not exist*/
#define O_EXCL 00000200   /*Fail if file already exists*/
#define O_NOCTTY 00000400 /*Do not assign controlling terminal*/

#define O_TRUNC 00001000 /*If the file exists and is a regular file,                    \
                            and the file is successfully opened O_RDWR or O_WRONLY, its \
                            length shall be truncated to 0*/

/*the file offset shall be set to the end of the file*/
#define O_APPEND 00002000
/*Non-blocking I/O mode*/
#define O_NONBLOCK 00004000

#define O_EXEC 00010000      /*Open for execute only(non-directory files)*/
#define O_SEARCH 00020000    /*Open directory for search only*/
#define O_DIRECTORY 00040000 /*must be a directory*/
#define O_NOFOLLOW 00100000  /*Do not follow symbolic links*/