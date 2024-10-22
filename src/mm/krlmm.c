#include "mmkit.h"

extern void test_vadr();
/* 初始化虚拟内存 */
GLOBVAR_DEFINITION(mmdsc_t, initmm);
void init_krlmm()
{
    init_kvirmemadrs();
    return;
}