#ifndef __SPONGE_COMM_H__
#define __SPONGE_COMM_H__

typedef enum {
  DBG_NONE  =  1 << 0,
  DBG_WARN  =  1 << 1,
  DBG_ERR   =  1 << 2,
  DBG_TRACE =  1 << 3
} DBG_LEVEL_e;

#define SPONGE_OK   0
#define SPONGE_FAIL -1

void dbg_mutex_init(void);
void dbg_printf(DBG_LEVEL_e dbglvl, const char* format, ...);

#endif