#ifndef _KRLGLOBAL_H_
#define _KRLGLOBAL_H_

#define KRL_DEFGLOB_VARIABLE(vartype,varname) \
KEXTERN  __attribute__((section(".data"))) vartype varname

#define HAL_DEFGLOB_VARIABLE(vartype,varname) \
EXTERN  __attribute__((section(".data"))) vartype varname

#endif // _KRLGLOBAL_H_