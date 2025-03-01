#include "usrinit.h"

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

int cat_command(int argc, char **argv) 
{
	int len = 0;
	char* filename = nullptr;
	int fd = 0;
	char* buf = nullptr;
	int i = 0, j = 0;

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
	color_printf(ORANGE,"length:%d\n", len);


	while(j < i)
		printf("%c", buf[j++]);
	
	printf("\n");
	
	close(fd);
    return 0;
}

int touch_command(int argc, char **argv) {
	char* filename = nullptr;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = open(filename, O_CREAT);

	return ret; 
}

int rm_command(int argc, char **argv) { 
	char* filename = nullptr;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = unlink(filename);

	return ret; 
}

int mkdir_command(int argc, char **argv) { 
	char* filename = nullptr;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = mkdir(filename);

	return ret; 
}

int rmdir_command(int argc, char **argv) { 
	char* filename = nullptr;
	int ret = 0;

	filename = get_filename_whole(filename, argv[1]); 

	ret = rmdir(filename);

	return ret; 
}

int exec_command(int argc, char **argv)
{
	int errno = 0;
	int retval = 0;
	char* filename = 0;

	errno = fork();

	if (errno == 0) {
		printf(" i am child -> [%d, %d] \n", getpid(), errno);
		// info('B');
	} else {
		printf(" i am parent -> [%d, %d] \n", getpid(), errno);
		// info('B');
	}
	while(1)
	{
		printf("[%#lx]: I am proc %d\n", retval, getpid());
        sleep(5);
        retval++;
	}

	if( errno ==  0 ) {
		// printf("child process\n");
		filename = get_filename_whole(filename, argv[1]);
		printf("exec_command filename:%s\n", filename);
		
		execve(filename, argv, nullptr);

		exit(0);
	} else {
		printf("parent process childpid:%#d\n", errno);
		waitpid(errno, &retval, 0);
		printf("parent process waitpid:%#018lx\n", retval);
	}

    return 0;
}

int reboot_command(int argc, char **argv) { return reboot(SYSTEM_REBOOT, nullptr); }

