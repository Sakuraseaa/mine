#ifndef _COMMOND_H__
#define _COMMOND_H__

void make_clear_abs_path(char *path, char *final_path);

int ls_command(int argc, char **argv);
int cd_command(int argc, char **argv);

int pwd_command(int argc, char **argv);
int cat_command(int argc, char **argv);
int touch_command(int argc, char **argv);
int rm_command(int argc, char **argv);
int mkdir_command(int argc, char **argv);
int rmdir_command(int argc, char **argv);
int exec_command(int argc, char **argv);
int reboot_command(int argc, char **argv);
int tree_command(int argc, char **argv);



#endif
