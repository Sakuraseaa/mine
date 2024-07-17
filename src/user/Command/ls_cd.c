#include "stat.h"
#include "time.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "dirent.h"
#include "string.h"

#define MAX_FILE_NAME_LEN 64
#define MAX_PATH_LEN 256

extern char* current_dir;
char qualifies[] = {'B', 'K', 'M', 'G', 'T'};

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
        else if (strcmp(".", name))
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

	// 给路径 申请缓冲
	i = len + strlen(argv[1]);
	path = malloc(i + 2, 0);
	memset(path, 0, i + 2);

	
	if(argv[1][0] == '/') // 是绝对路径
		strcpy(path, argv[1]);
	else 
		make_clear_abs_path(argv[1], path); // 是相对路径 把相对路径转换成绝对路径


	i = chdir(path);

	if(!i) {
		free(current_dir);
		current_dir = path;
	} else {
		free(path);
		printf("cd: %s:No such file or directory", argv[1]);
	}
	return 1;
}


static void strftime(time_t stamp, char *buf)
{
    tm time;
    localtime(stamp, &time);
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d",
            time.year,
            time.month,
            time.day,
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
			buf = (char*)malloc(512, 0);
		}
	}
	

	if(path[0] == '/') 
		dir = opendir(path);
	else {
		getcwd(current_dir, 256);
		dir = opendir(current_dir);
	}

	// printf("ls_command opendir:%d\n", dir->fd);
	entry = (struct dirent*)malloc(256, 0);
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

    return 1;
}

