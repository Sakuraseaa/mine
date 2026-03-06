#include "usrinit.h"

int analysis_keycode(int fd);
int read_line(int fd, char *buf);
void run_command(int index, int argc, char **argv);
int parse_command(char *buf, int *argc, char ***argv);
void print_prompt(void);

extern void * malloc(unsigned long size, int invalid);
extern unsigned int keycode_map_normal[NR_SCAN_CODES * MAP_COLS];

struct buildincmd
{
    char *cmd_name;
    int (*cmd_funcPtr)(int, char **);
};
char *current_dir = nullptr;
void sig_handler(long sig);


int main()
{
    long pid = getpid();

    pid = fork();
    if (pid == 0) {
        printf(" i am child -> [%d, %d] \n", getpid(), pid);
        info('B');
    } else {
        printf(" i am parent -> [%d, %d] \n", getpid(), pid);
        info('B');
        while(1) {
            sleep(5);
        }
    }

    // signal(4 , sig_handler);
    // kill(pid, 4);

    int fd = 0;
    char buf[256] = {0};
    char path[] = "/KEYBOARD.DEV";
    int index = -1;
    current_dir = (char*)malloc(2, 0);
    current_dir[0] = '/';
    current_dir[1] = 0;

    fd = open(path, 0);

    while (1)
    {
        int argc = 0;
        char **argv = nullptr;
        
        print_prompt();
        
        memset(buf, 0, 256);
        // 命令读取
        read_line(fd, buf);
        // 命令解析
        index = parse_command(buf, &argc, &argv);
        if (index < 0)
            printf("No Command Found!\n");
        else
            run_command(index, argc, argv); // 命令执行
    }

    free(current_dir);
    close(fd);

    while (1)
    {
        /* code */
    }
    // issue:: init进程有能力执行exit函数吗？ 有的需要编写一定的代码
    exit(0);
    }

void sig_handler(long sig) {
    printf("Catch signal is %d\n", sig);
}

void print_prompt(void) {

    color_printf(GREEN,"sk@mine");
    printf(":");
    color_printf(BLUE, "%s", current_dir);
    printf("$ ");
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
        unsigned int *keyrow = nullptr;
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
    {"date", date_command},
    {"echo", echo_command},
};

int find_cmd(char *cmd_name)
{
    int i = 0;
    for (; i < sizeof(shell_internal_cmd) / sizeof(struct buildincmd); i++)
    {
        if (!strcmp(cmd_name, shell_internal_cmd[i].cmd_name))
        {
            return i;
        }
    }
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
            printf("%s", buf);
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
        return -1;
    }

    if (!*argc)
        return -1;
    *argv = (char **)malloc(sizeof(char **) * (*argc), 0);
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
    free(argv);
}