#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "dirent.h"
#include "string.h"
#include "reboot.h"
#include "types.h"
#include "unistd.h"
#include "wait.h"

extern char* current_dir;

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

	len = strlen(current_dir);
	i = len + strlen(argv[1]);
	filename = malloc(i + 2, 0);
	memset(filename, 0, i + 2);
	strcpy(filename, current_dir);
	if(len > 1)
		filename[len] = '/';
	strcat(filename, argv[1]);
	// printf("cat_command filename:%s\n", filename);

	fd = open(filename, 0);
	i = lseek(fd, 0, SEEK_END);
	lseek(fd, 0 , SEEK_SET);
	buf = malloc(i + 1, 0);
	memset(buf, 0 , i + 1);
	len = read(fd, buf, i);
	printf("length:%d\t%s\n",len,buf);

	close(fd);
    return 0;
}
int touch_command(int argc, char **argv) {return 1; }
int rm_command(int argc, char **argv) { return 0; }
int mkdir_command(int argc, char **argv) { return 0; }
int rmdir_command(int argc, char **argv) { return 0; }
int exec_command(int argc, char **argv)
{
	int errno = 0;
	int retval = 0;
	int len = 0;
	char* filename = 0;
	int i = 0;

	errno = fork();
	if(errno ==  0 ) {
		printf("child process\n");
		len = strlen(current_dir);
		i = len + strlen(argv[1]);
		filename = malloc(i + 2, 0);
		memset(filename, 0, i + 2);
		strcpy(filename, current_dir);
		if(len > 1)
			filename[len] = '/';
		strcat(filename, argv[1]);

		printf("exec_command filename:%s\n", filename);
		for(i = 0; i < argc; i++)
			printf("argv[%d]:%s\n", i, argv[i]);
		
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

