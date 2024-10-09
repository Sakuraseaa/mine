#ifndef _DIR_F_H_
#define _DIR_F_H_

struct DIR* opendir(const char* path);
int closedir(struct DIR* dir);
struct dirent* readdir(struct DIR*dir);

#endif // _DIR_F_H_