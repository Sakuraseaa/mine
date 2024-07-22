#include "types.h"
#include "time.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "string.h"
#include "command.h"

extern char* current_dir;

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

int pwd_command(int argc, char **argv)
{	
    int ret = 0;
	getcwd(current_dir, strlen(current_dir));
	ret = printf(current_dir);
	printf("\n");
    return ret;
}

int echo_command(int argc, char **argv)
{
    int ret = 0, fd = 0;
    char* filename = NULL;
    
    filename = get_filename_whole(filename, argv[1]);
    
    if(argc == 4 && (strcmp(argv[3], ">>") == 0)) {
        
        fd = open(filename, O_RDWR | O_APPEND);
        ret = strlen(argv[4]);
        write(fd, argv[4], ret);

    } else {

        ret = printf("%s\n",argv[1]);
    }
    return ret;
}
