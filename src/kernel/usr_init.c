#include "wait.h"
#include "types.h"
#include "debug.h"
// #include "unistd.h"
#include "stdio.h"
#include "fcntl.h"
#include "keyboard.h"
#include "memory.h"
#include "init.h"
#include "dirent.h"
#include "signal.h"

extern int kill(long pid, long signum);
extern sighadler_t signal(long signum, sighadler_t handler);

int analysis_keycode(int fd);
int read_line(int fd, char *buf);
void run_command(int index, int argc, char **argv);
int parse_command(char *buf, int *argc, char ***argv);

extern unsigned int keycode_map_normal[NR_SCAN_CODES * MAP_COLS];

struct buildincmd
{
	char *cmd_name;
	int (*cmd_funcPtr)(int, char **);
};
char *current_dir = NULL;
int sk = 0;

static void handler(long sig) {
	
	printf("The signal is %d\n", sig);

}
extern unsigned long volatile jiffies;
extern unsigned long startup_time;
#include "time.h"

void test_signal() {
	signal(2, &handler);
	long pid = getpid();
	kill(pid, 2);
}

void test_time() {
	// 等待5秒
	sleep(5);
    struct time ttmm;
    memset(&ttmm, 0, sizeof(struct time));
    localtime(startup_time + (jiffies / 100), &ttmm);
    printf("year:%#010d, month:%#010d, day:%#010d,  week:%#010d, hour:%#010d, mintue:%#010d, second:%#010d\n",
    ttmm.year, ttmm.month, ttmm.day, ttmm.week_day, ttmm.hour, ttmm.minute, ttmm.second);
}

#include "VFS.h"
void test_minix() {
	
	change_fs();
	
	char Path[] = "/if.c";
	char mem[5] = {0};
	int fd = open(Path, 0);
	read(fd, mem, 1);
	lseek(fd, 0, SEEK_END);
	write(fd, mem, 1);
	
	change_fs();
}

int usr_init()
{
	test_minix();

	int fd = 0;
	unsigned char buf[256] = {0};
	char path[] = "/KEYBOARD.DEV";
	int index = -1;
	current_dir = "/";
	fd = open(path, 0);

	while (1)
	{
		sk++;
		int argc = 0;
		char **argv = NULL;
		printf("sk@Mine %d #:", sk);
		memset(buf, 0, 256);
		// 命令读取
		read_line(fd, buf);
		// 命令解析
		index = parse_command(buf, &argc, &argv);
		if (index < 0)
			printf("Input Error, No Command Found!\n");
		else
			run_command(index, argc, argv); // 命令执行
	}

	close(fd);

	while (1)
		;
	return 0;
}

/**
 * @brief 从键盘文件中读取键盘扫描码
 *
 * @param fd 键盘文件描述符
 * @return unsigned char
 */
unsigned char get_scancode(int fd)
{
	unsigned char ret = 0;
	read(fd, &ret, 1);
	return ret;
}

/**
 * @brief 解析键盘扫描码为对应的字符
 *
 * @param fd   键盘文件描述符
 * @return int
 */
int analysis_keycode(int fd)
{
	unsigned char x = 0;
	int i;
	int key = 0;
	int make = 0;

	x = get_scancode(fd);

	if (x == 0xE1) // pause break;
	{
		key = PAUSEBREAK;
		for (i = 1; i < 6; i++)
			if (get_scancode(fd) != pausebreak_scode[i])
			{
				key = 0;
				break;
			}
	}
	else if (x == 0xE0) // print screen
	{
		x = get_scancode(fd);

		switch (x)
		{
		case 0x2A: // press printscreen

			if (get_scancode(fd) == 0xE0)
				if (get_scancode(fd) == 0x37)
				{
					key = PRINTSCREEN;
					make = 1;
				}
			break;

		case 0xB7: // UNpress printscreen

			if (get_scancode(fd) == 0xE0)
				if (get_scancode(fd) == 0xAA)
				{
					key = PRINTSCREEN;
					make = 0;
				}
			break;

		case 0x1d: // press right ctrl

			ctrl_r = 1;
			key = OTHERKEY;
			break;

		case 0x9d: // UNpress right ctrl

			ctrl_r = 0;
			key = OTHERKEY;
			break;

		case 0x38: // press right alt

			alt_r = 1;
			key = OTHERKEY;
			break;

		case 0xb8: // UNpress right alt

			alt_r = 0;
			key = OTHERKEY;
			break;

		default:
			key = OTHERKEY;
			break;
		}
	}

	if (key == 0)
	{
		unsigned int *keyrow = NULL;
		int column = 0;

		make = (x & FLAG_BREAK ? 0 : 1);

		keyrow = &keycode_map_normal[(x & 0x7F) * MAP_COLS];

		if (shift_l || shift_r)
			column = 1;

		key = keyrow[column];

		switch (x & 0x7F)
		{
		case 0x2a: // SHIFT_L:
			shift_l = make;
			key = 0;
			break;

		case 0x36: // SHIFT_R:
			shift_r = make;
			key = 0;
			break;

		case 0x1d: // CTRL_L:
			ctrl_l = make;
			key = 0;
			break;

		case 0x38: // ALT_L:
			alt_l = make;
			key = 0;
			break;

		case 0x01: // ESC
			key = 0;
			break;

		default:
			if (!make)
				key = 0;
			break;
		}

		if (key)
			return key;
	}
	return 0;
}

int cd_command(int argc, char **argv) 
{
	char* path = NULL;
	int len = 0;
	int i = 0;
	len = strlen(current_dir);

	if(!strcmp(".", argv[1])) return 1;

	if(!strcmp("..", argv[1]))
	{
		if(!strcmp("/", current_dir))
			return 1;
		for(i = len - 1; i > 1; i--)
			if(current_dir[i] == '/')
				break;
		current_dir[i] = '\0';
		printf("pwd switch to %s\n", current_dir);
		return 1;
	}

	i = len + strlen(argv[1]);
	path = kmalloc(i + 2, 0);
	memset(path, 0, i + 2);
	strcpy(path, current_dir);
	if(len > 1)
		path[len] = '/';
	strcat(path, argv[1]);
	printf("cd_command:%s\n", path);

	i = chdir(path);
	if(!i)
		current_dir = path;
	else
		printf("Can't Goto Dir %s\n", argv[1]);
	printf("pwd switch to %s\n", current_dir);
}

#include "stat.h"
int ls_command(int argc, char **argv) 
{
	struct DIR* dir = NULL;
	struct dirent* buf = NULL;
	stat_t  statbuf;
	dir=opendir(current_dir);
	// printf("ls_command opendir:%d\n", dir->fd);

	buf = (struct dirent*)kmalloc(256, 0);
	// 直到该目录为空
	while(1)
	{
		buf = readdir(dir);// 每次读一条目录项
		if(buf == NULL)
			break;
		// fstat(dir->fd, &statbuf);
		// 打印信息
		printf("%s\t", buf->d_name);
	}
	printf("\n");
	closedir(dir);
}

int pwd_command(int argc, char **argv)
{
	printf(current_dir);
	printf("\n");
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
	filename = kmalloc(i + 2, 0);
	memset(filename, 0, i + 2);
	strcpy(filename, current_dir);
	if(len > 1)
		filename[len] = '/';
	strcat(filename, argv[1]);
	// printf("cat_command filename:%s\n", filename);

	fd = open(filename, 0);
	i = lseek(fd, 0, SEEK_END);
	lseek(fd, 0 , SEEK_SET);
	buf = kmalloc(i + 1, 0);
	memset(buf, 0 , i + 1);
	len = read(fd, buf, i);
	printf("length:%d\t%s\n",len,buf);

	close(fd);
}
int touch_command(int argc, char **argv) { return 0;}
int rm_command(int argc, char **argv) { return 0; }
int mkdir_command(int argc, char **argv) { return 0;}
int rmdir_command(int argc, char **argv) { return 0;}
int exec_command(int argc, char **argv)
{
	int errno = 0;
	long retval = 0;
	int len = 0;
	char* filename = 0;
	int i = 0;

	errno = fork();
	if(errno ==  0 ) {
		printf("child process\n");
		len = strlen(current_dir);
		i = len + strlen(argv[1]);
		filename = kmalloc(i + 2, 0);
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
	return errno;
}
int reboot_command(int argc, char **argv) { return reboot(SYSTEM_REBOOT, NULL); }

struct buildincmd shell_internal_cmd[] =
	{
		{"cd", cd_command},
		{"ls", ls_command},
		{"pwd", pwd_command},
		{"cat", cat_command},
		{"touch", touch_command},
		{"rm", rm_command},
		{"mkdir", mkdir_command},
		{"rmdir", rmdir_command},
		{"exec", exec_command},
		{"reboot", reboot_command},
};

int find_cmd(char *cmd_name)
{
	int i = 0;
	for (; i < sizeof(shell_internal_cmd) / sizeof(struct buildincmd); i++)
		if (!strcmp(cmd_name, shell_internal_cmd[i].cmd_name))
			return i;
	return -1;
}

int read_line(int fd, char *buf)
{
	char key = 0;
	int count = 0;

	while (1)
	{
		key = analysis_keycode(fd);

		switch (key)
		{
		case 0: // 通码
			break;
		case '\n':
			printf("\n");
			buf[count] = 0;
			return count;
		case '\b':
			if (count)
			{
				count--;
				printf("%c", key);
			}
			break;
		default:
			buf[count++] = key;
			printf("%c", key);
			break;
		}
	}
}

int parse_command(char *buf, int *argc, char ***argv)
{
	int i = 0, j = 0;

	// 越过前导空格
	while (buf[j] == ' ')
		j++;

	// 统计参数个数
	for (i = j; i < 256; i++)
	{
		if (!buf[i])
			break;
		if (buf[i] != ' ' && (buf[i + 1] == ' ' || buf[i + 1] == '\0'))
			(*argc)++;
	}
	// printf("parse_common argc: %d\n", *argc);

	if (!*argc)
		return -1;
	*argv = (char **)kmalloc(sizeof(char **) * (*argc), 0);
	// printf("parse_command argv:%#018lx, *argv:%#018lx\n", argv, *argv);

	for (i = 0; i < *argc && j < 256; i++)
	{
		*((*argv) + i) = &buf[j];
		while (buf[j] && buf[j] != ' ')
			j++;
		buf[j++] = '\0';
		while (buf[j] == ' ')
			j++;
		// printf("%s\n", (*argv)[i]);
	}

	return find_cmd(**argv);
}

void run_command(int index, int argc, char **argv)
{
	// printf("run_command %s\n", shell_internal_cmd[index].cmd_name);
	shell_internal_cmd[index].cmd_funcPtr(argc, argv);
}
