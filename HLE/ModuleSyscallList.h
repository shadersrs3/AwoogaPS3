#ifndef _AWOOGAPS3_MODULE_SYSCALL_LIST_H
#define _AWOOGAPS3_MODULE_SYSCALL_LIST_H

#include <HLE/HLE.h>

extern HLEImportModule *get_sceNpTus();
extern HLEImportModule *get_sceNp();
extern HLEImportModule *get_sceNp2();
extern HLEImportModule *get_cellNetCtl();
extern HLEImportModule *get_sys_net();
extern HLEImportModule *get_sys_fs();
extern HLEImportModule *get_cellGameExec();
extern HLEImportModule *get_cellGame();
extern HLEImportModule *get_cellGcmSys();
extern HLEImportModule *get_cellSync();
extern HLEImportModule *get_cellSpurs();
extern HLEImportModule *get_cellFiber();
extern HLEImportModule *get_cellSysutil();
extern HLEImportModule *get_sysPrxForUser();
extern HLEImportModule *get_cellSysmodule();

extern void register_syscall_sys_timer_functions();
extern void register_syscall_sys_event_functions();
extern void register_syscall_sys_spu_functions();
extern void register_syscall_sys_time_functions();
extern void register_syscall_sys_memory_functions();
extern void register_syscall_sys_dbg_functions();
extern void register_syscall_sys_tty_functions();
extern void register_syscall_sys_ppu_thread_functions();


#endif