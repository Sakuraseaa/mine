#include "toolkit.h"
#include "fskit.h"
#include "wait.h"
#include "keyboard.h"
#include "mmkit.h"
#include "init.h"
#include "dirent.h"
#include "signal.h"

#define WHITE 0x00ffffff  // 白
#define BLACK 0x00000000  // 黑
#define RED 0x00ff0000	  // 红
#define ORANGE 0x00ff8000 // 橙
#define YELLOW 0x00ffff00 // 黄
#define GREEN 0x0000ff00  // 绿
#define BLUE 0x000000ff	  // 蓝
#define INDIGO 0x0000ffff // 靛
#define PURPLE 0x008000ff // 紫
extern int color_printf(unsigned int FRcolor, const char* fmt, ...);
extern int kill(long pid, long signum);
extern sighadler_t signal(long signum, sighadler_t handler);

int analysis_keycode(int fd);
int read_line(int fd, char *buf);
void run_command(int index, int argc, char **argv);
int parse_command(char *buf, int *argc, char ***argv);

static char* get_filename_whole(char* buf, char* reletive_path);
extern unsigned int keycode_map_normal[NR_SCAN_CODES * MAP_COLS];

struct buildincmd
{
	char *cmd_name;
	int (*cmd_funcPtr)(int, char **);
};
char *current_dir = NULL;
static void handler(long sig) {
	
	printf("The signal is %d\n", sig);

}
extern unsigned long volatile jiffies;
extern unsigned long startup_time;

static void print_prompt(void) {
	
	color_printf(GREEN,"sk@Mine");
	printf(":");
	color_printf(BLUE, "%s", current_dir);
	printf("$ ");
}

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

int usr_init()
{

	int fd = 0;
	char buf[256] = {0};
	char path[] = "/KEYBOARD.DEV";
	int index = -1;
	current_dir = (char*)knew(2, 0);
	current_dir[0] = '/';
	current_dir[1] = 0;

	fd = open(path, 0);

	while (1)
	{
		int argc = 0;
		char **argv = NULL;
		
		print_prompt();
		
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

	kdelete(current_dir, strlen(current_dir));
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
		
		if(ctrl_l && (key == 'l')) // ctrl_l + l 是清屏幕的命令
			key = 'l' - 'a';

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

#define MAX_FILE_NAME_LEN 64
#define MAX_PATH_LEN 256

/**
 * @brief path_parse用于获得文件路径pathname的顶层路径, 顶层路径存入到name_store中
 *
 * @example pathname="/home/sk", char name_store[10]
 *          path_parse(pathname, name_store) -> return "/sk", *name_store="home"
 *
 * @param pathname 需要解析的文件路径
 * @param name_store 主调函数提供的缓冲区
 * @return char* 指向除顶层路径之外的子路径字符串的地址
 */
static char *path_parse(char *pathname, char *name_store)
{
    // 根目录不需要解析, 跳过即可
    if (pathname[0] == '/')
        while (*(++pathname) == '/')
            ; // 跳过'//a', '///b'

    while (*pathname != '/' && *pathname != 0)
        *name_store++ = *pathname++;

    if (pathname[0] == 0) // pathname为空, 则表示路径已经结束了, 此时返回NULL
        return NULL;

    return pathname;
}
/**
 * @brief wash_path用于将包含相对路径的old_path转换为绝对路径后存入new_abs_path.
 *        例如将 /a/b/../c/./d 转换为/a/c/d
 *
 * @param old_abs_path 包含相对路径的old_path
 * @param new_abs_path 新的绝对路径
 */
static void wash_path(char *old_abs_path, char *new_abs_path)
{
    assert(old_abs_path[0] == '/');

    char name[256] = {0};
    char *sub_path = old_abs_path;
    sub_path = path_parse(sub_path, name);
    // 只输入了 '/'
    if (name[0] == 0)
    {
        new_abs_path[0] = '/';
        new_abs_path[1] = 0;
        return;
    }

    // 将new_abs_path "清空"
    new_abs_path[0] = 0;
    // 拼接根目录
    strcat(new_abs_path, "/");

    // 逐层向下遍历目录
    while (name[0])
    {
        // 如果当前目录是上级目录，则寻找上一个'/',然后删除上一个'/'的内容
        // 比如‘/a/b/..’ 设置为 ‘/a’
        if (!strcmp("..", name))
        {
            char *slash_ptr = strrchr(new_abs_path, '/');
            // 如果没有找到根目录的'/', 则截断
            if (slash_ptr != new_abs_path)
                *slash_ptr = 0;
            // 如果已经找到了根目录, 则截断为'/0xxxxx'
            else
                *(slash_ptr + 1) = 0;
            // 当前目录不是 '.' ,就将name拼接到new_abs_path
        }
        else if (strcmp(".", name) != 0)
        {
            if (strcmp(new_abs_path, "/")) // 如果new_abs_path不是"/",就拼接一个"/",此处的判断是为了避免路径开头变成这样"//"
                strcat(new_abs_path, "/");

            strcat(new_abs_path, name);
        }

        // 准备下次一遍历
        memset(name, 0, MAX_FILE_NAME_LEN);
        if (sub_path)
            sub_path = path_parse(sub_path, name);
    }
}

/**
 * @brief make_clear_abs_path用于将包含相对路径的目录path('.'和'..')处理不含相对路径的目录, 并存入final_path中
 *        使用系统调用获得(当前工作目录) + path, 使用wash_path删除多于的 . 和 ..

 * @param path 用户传入的绝对路径
 * @param final_path  不包含相对路径的目录
 */
void make_clear_abs_path(char *path, char *final_path)
{
    char abs_path[MAX_PATH_LEN] = {0};

    if (path[0] != '/')
    {
        memset(abs_path, 0, sizeof(abs_path));
        if (getcwd(abs_path, MAX_PATH_LEN))
        {
            if (!(abs_path[0] == '/' && abs_path[1] == 0))
                strcat(abs_path, "/");
        }
    }
    strcat(abs_path, path);
    wash_path(abs_path, final_path);
}

int cd_command(int argc, char **argv) 
{
	char* path = NULL;
	int len = 0;
	int i = 0;
	len = strlen(current_dir);

	if(strcmp(".", argv[1]) == 0) return 1;

	if(strcmp("..", argv[1]) == 0)
	{
		if(!strcmp("/", current_dir))
			return 1;
		for(i = len - 1; i > 1; i--)
			if(current_dir[i] == '/')
				break;
		current_dir[i] = '\0';
		i = chdir(current_dir);
		return i;
	}

	path = get_filename_whole(path, argv[1]);

	i = chdir(path);

	if(!i) {
		kdelete(current_dir, strlen(current_dir));
		current_dir = path;
	} else {
		kdelete(path, strlen(path));
		color_printf(RED, "cd: %s:No such file or directory\n", argv[1]);
	}
	return 1;
}
#include "fskit.h"
static void strftime(time_t stamp, char *buf)
{
    tm time;
    localtime(stamp, &time);
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d",
            time.year,
            time.month,
            time.week_day,
            time.hour,
            time.minute,
            time.second);
}

static void parsemode(int mode, char *buf)
{
    memset(buf, '-', 10);
    buf[10] = '\0';
    char *ptr = buf;

    switch (mode & IFMT)
    {
    case IFREG:
        *ptr = '-';
        break;
    case IFBLK:
        *ptr = 'b';
        break;
    case IFDIR:
        *ptr = 'd';
        break;
    case IFCHR:
        *ptr = 'c';
        break;
    case IFIFO:
        *ptr = 'p';
        break;
    case IFLNK:
        *ptr = 'l';
        break;
    case IFSOCK:
        *ptr = 's';
        break;
    default:
        *ptr = '?';
        break;
    }
    ptr++;

    for (int i = 6; i >= 0; i -= 3)
    {
        int fmt = (mode >> i) & 07;
        if (fmt & 0b100)
        {
            *ptr = 'r';
        }
        ptr++;
        if (fmt & 0b010)
        {
            *ptr = 'w';
        }
        ptr++;
        if (fmt & 0b001)
        {
            *ptr = 'x';
        }
        ptr++;
    }
}

char qualifies[] = {'B', 'K', 'M', 'G', 'T'};

void reckon_size(int *size, char *qualifer)
{
    int num = *size;
    int i = 0;
    *qualifer = 'B';

    while (num)
    {
        *qualifer = qualifies[i];
        *size = num;
        num >>= 10; // num /= 1024
        i++;
    }
}
int ls_command(int argc, char **argv) 
{
	struct DIR* dir = NULL;
	struct dirent* entry = NULL;
	char* buf  = NULL;
	char* path = NULL;
	stat_t  statbuf;
	bool isDetail = false, isDirectory = false;
	for(size_t i = 0; i < argc; i++) {
		char* str = argv[i];

		if(str[0] != '-')		// 记录字符串
			path = str;

		if(strcmp(str, "-l") == 0){  // 检测 -l 标志
			isDetail = true;
			buf = (char*)knew(512, 0);
		}
	}
	
	assert(path != NULL);

	if(path[0] == '/') 
		dir = opendir(path);
	else {
		getcwd(current_dir, strlen(current_dir));
		dir = opendir(current_dir);
	}

	// printf("ls_command opendir:%d\n", dir->fd);
	entry = (struct dirent*)knew(256, 0);
	// 直到该目录为空
	while(1)
	{
		entry = readdir(dir);// 每次读一条目录项
		if(entry == NULL)
			break;
		if(entry->d_name[0] == '.') // 跳过隐藏文件
			continue;
		if(!isDetail) {
			printf("%s\t", entry->d_name);
		}
		if(isDetail == false)
			continue;

		stat(entry->d_name, &statbuf);

        parsemode(statbuf.mode, buf);
        printf("%s ", buf);
		
		if(buf[0] == 'd')	// 标记此项为目录
			isDirectory = true;

        strftime(statbuf.ctime, buf);

        int size = statbuf.size;
        char qualifier;
        reckon_size(&size, &qualifier);

        printf("%-2d %-4d %-4d %-d%-c\t%-s  ",
               statbuf.nlinks,
               statbuf.uid,
               statbuf.gid,
               size,
               qualifier,
               buf);
		
		if(isDirectory == false)
			printf("%-s\n", entry->d_name);
		else
			color_printf(INDIGO,"%-s\n",entry->d_name);
		
		isDirectory = false;
	}
	printf("\n");
	closedir(dir);
	kdelete(entry, 256);
	return 0;
}

extern int errno;

int pwd_command(int argc, char **argv)
{	
	getcwd(current_dir, strlen(current_dir));
	printf(current_dir);
	printf("\n");
	return 0;
}
int cat_command(int argc, char **argv) 
{
	int len = 0;
	char* filename = NULL;
	int fd = 0;
	char* buf = NULL;
	int i = 0, j = 0;

	// 拼凑绝对路径
	len = strlen(current_dir);
	i = len + strlen(argv[1]);
	filename = knew(i + 2, 0);
	memset(filename, 0, i + 2);

	if(argv[1][0] == '/') // 是绝对路径
		strcpy(filename, argv[1]);
	else 
		make_clear_abs_path(argv[1], filename); // 是相对路径 把相对路径转换成绝对路径

	fd = open(filename, 0);
	
	if(errno == -ENOENT) {
	
		color_printf(RED, "cat: %s:No such file or directory\n", argv[1]);
		return -1;
	}
	i = lseek(fd, 0, SEEK_END);
	lseek(fd, 0 , SEEK_SET);
	buf = knew(i + 1, 0);
	memset(buf, 0 , i + 1);
	len = read(fd, buf, i);
	printf("length:%d\n",len);

	while(j < i)
		printf("%c", buf[j++]);
	
	printf("\n");
	
	close(fd);
    return 0;
}

int touch_command(int argc, char **argv) { 
	return 0;
}

// ==========Update==========
int info_command(int argc, char **argv){
    for (size_t i = 1; i < argc; i++)
    {
        if(!strcmp("tree", argv[i]))
            info('A');
        else if(!strcmp("tab", argv[i]))
            info('B');
    }
	return 0;
}
// =======================

int rm_command(int argc, char **argv) { return 0; }
int mkdir_command(int argc, char **argv) { return 0;}
int rmdir_command(int argc, char **argv) { return 0;}

static char* get_filename_whole(char* buf, char* reletive_path) {

	int len = 0;
	// 拼凑绝对路径
	len = strlen(current_dir);
	len = len + strlen(reletive_path);
	buf = knew(len + 2, 0);
	memset(buf, 0, len + 2);

	make_clear_abs_path(reletive_path, buf); // 是相对路径 把相对路径转换成绝对路径

	return buf;
}

int exec_command(int argc, char **argv)
{
	char* filename = 0;

	printf("child process\n");
	filename = get_filename_whole(filename, argv[1]);
	printf("exec_command filename:%s\n", filename);
	

	return errno;
}
int reboot_command(int argc, char **argv) { return reboot(SYSTEM_REBOOT, NULL); }
int echo_command(int argc, char **argv) {
    int ret = 0, fd = 0;
    char* filename = NULL;
    
    if(argc == 4 && (strcmp(argv[2], ">>") == 0)) {
        
		filename = get_filename_whole(filename, argv[3]);
        fd = open(filename, O_RDWR | O_APPEND);
        ret = strlen(argv[1]);
        write(fd, argv[1], ret);
		close(fd);
	
    } else {

        ret = printf("%s\n",argv[1]);
    }
    return ret;
}
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
		{"info", info_command},
		{"echo", echo_command},
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
		case 'l' - 'a':
			cleanScreen();
			print_prompt();
			printf(buf);
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
	bool is_cmpstr_ing = false;
	// 越过前导空格
	while (buf[j] == ' ')
		j++;

	// 统计参数个数 , 此处的识别有问题需要重写
	for (i = j; i < 256; i++)
	{		
		if (!buf[i])
			break;

		if(is_cmpstr_ing == false && (buf[i] == '\"' || buf[i] == '\'')) {
			is_cmpstr_ing = true;	// 开始引号匹配
			continue;
		}
		
		if(is_cmpstr_ing == true && (buf[i] == '\"' || buf[i] == '\'')) {
			is_cmpstr_ing = 1 - is_cmpstr_ing; // 结束引号匹配
			(*argc)++;
			continue;
		}
		
		if(is_cmpstr_ing)
			continue;

		if (buf[i] != ' ' && (buf[i + 1] == ' ' || buf[i + 1] == '\0'))
			(*argc)++;
	}

	// 说明 ' 和 " 没有成对的匹配，
	if(is_cmpstr_ing) {
		printf(" \' or \" not a perfect mathc\n");
		return 0;
	}

	if (!*argc)
		return -1;
	*argv = (char **)knew(sizeof(char **) * (*argc), 0);
	// printf("parse_command argv:%#018lx, *argv:%#018lx\n", argv, *argv);

	for (i = 0; i < *argc && j < 256; i++)
	{	
		if(buf[j] == '\'' || buf[j] == '\"') {
			j++;
			is_cmpstr_ing = true;
		}

		*((*argv) + i) = &buf[j];

		// 这里有5个 && 有些许丑陋了  🥺 😖
		while ((is_cmpstr_ing == false) && (buf[j] && buf[j] != ' '))
			j++;

		while((is_cmpstr_ing && buf[j] ) && (buf[j] != '\'' && buf[j] != '\"'))
			j++;
		

		buf[j++] = '\0';
	
		while (buf[j] == ' ')
			j++;

		is_cmpstr_ing = false;
	}

	return find_cmd(**argv);
}

void run_command(int index, int argc, char **argv)
{
	// printf("run_command %s\n", shell_internal_cmd[index].cmd_name);
	shell_internal_cmd[index].cmd_funcPtr(argc, argv);
}
