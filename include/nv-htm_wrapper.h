#ifndef NVHTM_WRAPPER_H
#define NVHTM_WRAPPER_H
#  ifdef NVHTM
#    include "nvhtm.h"
#    define NVHTM_begin() NH_begin()
#    define NVHTM_end() NH_commit()
#    define NVM_write(p, val) NH_write(p, val)
#    define NVM_read(p) NH_read(p)
#  else
#    define NVHTM_begin()
#    define NVHTM_end()
#    define NVM_write(p, val) *p = val
#    define NVM_read(p) *p
#  endif
#endif
