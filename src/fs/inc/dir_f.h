#ifndef _DIR_F_H_
#define _DIR_F_H_

struct DIR* opendir(cstr_t path);
s32_t closedir(struct DIR* dir);
struct dirent* readdir(struct DIR*dir);

#endif // _DIR_F_H_