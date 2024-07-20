#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "dirent.h"
#include "string.h"
#include "reboot.h"
#include "types.h"
#include "unistd.h"
#include "wait.h"
#include "errno.h"
#include "command.h"
#include "printf.h"
#include "time.h"
#include "fcntl.h"

extern char* current_dir;

char* get_filename_whole(char* buf, char* reletive_path) {

	int len = 0;
	// 拼凑绝对路径
	len = strlen(current_dir);
	len = len + strlen(reletive_path);
	buf = malloc(len + 2, 0);
	memset(buf, 0, len + 2);

	make_clear_abs_path(reletive_path, buf); // 是相对路径 把相对路径转换成绝对路径

	return buf;
}

int pwd_command(int argc, char **argv)
{	
    int ret = 0;
	getcwd(current_dir, strlen(current_dir));
	ret = printf(current_dir);
	printf("\n");
    return ret;
}
int cat_command(int argc, char **argv) 
{
	int len = 0;
	char* filename = NULL;
	int fd = 0;
	char* buf = NULL;
	int i = 0;

	filename = get_filename_whole(filename, argv[1]); 

	fd = open(filename, 0);
	
	if(errno == -ENOENT) {
	
		color_printf(RED, "cat: %s:No such file or directory\n", argv[1]);
		return -1;
	}
	i = lseek(fd, 0, SEEK_END);
	lseek(fd, 0 , SEEK_SET);
	buf = malloc(i + 1, 0);
	memset(buf, 0 , i + 1);
	len = read(fd, buf, i);
	printf("length:%d\t%s\n",len,buf);

	close(fd);
    return 0;
}
int touch_command(int argc, char **argv) {
	char* filename = NULL;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = open(filename, O_CREAT);

	return ret; 
}
int rm_command(int argc, char **argv) { 
	char* filename = NULL;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = unlink(filename);

	return ret; 
}
int mkdir_command(int argc, char **argv) { 
	char* filename = NULL;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = mkdir(filename);

	return ret; 
}
int rmdir_command(int argc, char **argv) { return 0; }
int exec_command(int argc, char **argv)
{
	int errno = 0;
	int retval = 0;
	char* filename = 0;

	errno = fork();
	if( errno ==  0 ) {
		// printf("child process\n");
		filename = get_filename_whole(filename, argv[1]);
		printf("exec_command filename:%s\n", filename);
		
		execve(filename, argv, NULL);

		exit(0);
	} else {
		printf("parent process childpid:%#d\n", errno);
		waitpid(errno, &retval, 0);
		printf("parent process waitpid:%#018lx\n", retval);
	}
    return 0;
}
int reboot_command(int argc, char **argv) { return reboot(SYSTEM_REBOOT, NULL); }

int tree_command(int argc, char **argv) {
	return tree();
}
// Monday Tuesday Wednesday Thursday Friday Saturday Sunday
static const char* week[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

//January February March April May June July August September October November December
static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};



int date_command(int argc, char **argv)
{
	u64 seconds = getNow();
    tm time;
    localtime(seconds, &time);
    printf("%d-%02d-%02d %s-%s %02d:%02d:%02d \n",
            time.year,
            time.month,
            time.day,
			months[time.month - 1],
			week[time.week_day - 1],
            time.hour,
            time.minute,
            time.second);
	return 1;
}

