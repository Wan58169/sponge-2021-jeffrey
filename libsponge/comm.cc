#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include "comm.hh"

static pthread_mutex_t dbglk;
static DBG_LEVEL_e glb_dbglvl = DBG_ERR;

void dbg_mutex_init(void)
{
  pthread_mutex_init(&dbglk, NULL);
}

void dbg_printf(DBG_LEVEL_e dbglvl, const char* format, ...)
{
  va_list args;
  char buf[256];

  if(!(glb_dbglvl & dbglvl)) {
    return;
  }

  pthread_mutex_lock(&dbglk);
  va_start(args, format);
  vsnprintf(buf, 256, format, args);
  va_end(args);

  printf("%s", buf);
  pthread_mutex_unlock(&dbglk);
}