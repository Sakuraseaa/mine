#ifndef _COMMOND_H__
#define _COMMOND_H__

void make_clear_abs_path(char *path, char *final_path);
char* get_filename_whole(char* buf, char* reletive_path);


int ls_command(int argc, char **argv);
int cd_command(int argc, char **argv);
int pwd_command(int argc, char **argv);
int cat_command(int argc, char **argv);
int echo_command(int argc, char **argv);
int touch_command(int argc, char **argv);
int rm_command(int argc, char **argv);
int mkdir_command(int argc, char **argv);
int rmdir_command(int argc, char **argv);
int exec_command(int argc, char **argv);
int reboot_command(int argc, char **argv);
int info_command(int argc, char **argv);
int date_command(int argc, char **argv);


#endif
