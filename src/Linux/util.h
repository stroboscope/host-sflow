/* This software is distributed under the following license:
 * http://host-sflow.sourceforge.net/license.html
 */

#ifndef UTIL_H
#define UTIL_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <syslog.h>
#include <assert.h>

#include <sys/types.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // for PRIu64 etc.
#include "malloc.h" // for malloc_stats()

#include <sys/wait.h>
#include <ctype.h> // for isspace() etc.
#include "pthread.h"

#include <sys/time.h> // for timeradd()

#include "sys/syscall.h" /* just for gettid() */
#define MYGETTID (pid_t)syscall(SYS_gettid)

#define YES 1
#define NO 0

#include "sflow.h" // for SFLAddress, SFLAdaptorList...

  extern int debug;
  
  // addressing
  int lookupAddress(char *name, struct sockaddr *sa, SFLAddress *addr, int family);
  int parseNumericAddress(char *name, struct sockaddr *sa, SFLAddress *addr, int family);
  int hexToBinary(u_char *hex, u_char *bin, uint32_t binLen);
  int printHex(const u_char *a, int len, u_char *buf, int bufLen, int prefix);
  int parseUUID(char *str, char *uuid);
  int printUUID(const u_char *a, u_char *buf, int bufLen);
  uint32_t hashUUID(char *uuid);
  int printSpeed(const uint64_t speed, char *buf, int bufLen);
  
  // logger
  void myLog(int syslogType, char *fmt, ...);

  // OS allocation
  void *my_os_calloc(size_t bytes);
  void *my_os_realloc(void *ptr, size_t bytes);
  void my_os_free(void *ptr);

#define SYS_CALLOC calloc
#define SYS_REALLOC realloc
#define SYS_FREE free

#ifdef UTHEAP
  // realm allocation (buffer recycling)
  void UTHeapInit(void);
  void *UTHeapQNew(size_t len);
  void *UTHeapQReAlloc(void *buf, size_t newSiz);
  void UTHeapQFree(void *buf);
  void UTHeapGC(void);

#define my_calloc UTHeapQNew
#define my_realloc UTHeapQReAlloc
#define my_free UTHeapQFree
#else
#define my_calloc my_os_calloc
#define my_realloc my_os_realloc
#define my_free my_os_free
#endif

  // desync'd timer-tick support
  void UTClockDesync_uS(uint32_t uS);
  time_t UTClockSeconds(void);

  // safer string fns
  uint32_t my_strnlen(const char *s, uint32_t max);
  uint32_t my_strlen(const char *s);
  char *my_strdup(char *str);
  int my_strnequal(char *s1, char *s2, uint32_t max);
  int my_strequal(char *s1, char *s2);
  uint32_t my_strhash(char *str);

  // mutual-exclusion semaphores
  static inline int lockOrDie(pthread_mutex_t *sem) {
    if(sem && pthread_mutex_lock(sem) != 0) {
      myLog(LOG_ERR, "failed to lock semaphore!");
      exit(EXIT_FAILURE);
    }
    return YES;
  }

  static inline int releaseOrDie(pthread_mutex_t *sem) {
    if(sem && pthread_mutex_unlock(sem) != 0) {
      myLog(LOG_ERR, "failed to unlock semaphore!");
      exit(EXIT_FAILURE);
    }
    return YES;
  }

#define STRINGIFY(Y) #Y
#define STRINGIFY_DEF(D) STRINGIFY(D)

#define DYNAMIC_LOCAL(VAR) VAR
#define SEMLOCK_DO(_sem) for(int DYNAMIC_LOCAL(_ctrl)=1; DYNAMIC_LOCAL(_ctrl) && lockOrDie(_sem); DYNAMIC_LOCAL(_ctrl)=0, releaseOrDie(_sem))

  // string utils
  char *trimWhitespace(char *str);
  void setStr(char **fieldp, char *str);

  // string buffer
  typedef struct _UTStrBuf {
    char *buf;
    size_t len;
    size_t cap;
  } UTStrBuf;

  UTStrBuf *UTStrBuf_new(size_t cap);
  void UTStrBuf_grow(UTStrBuf *buf);
  void UTStrBuf_append(UTStrBuf *buf, char *str);
  int UTStrBuf_printf(UTStrBuf *buf, char *fmt, ...);
  char *UTStrBuf_unwrap(UTStrBuf *buf);

  // string array
  typedef struct _UTStringArray {
    char **strs;
    uint32_t n;
    uint32_t capacity;
    int8_t sorted;
  } UTStringArray;

  UTStringArray *strArrayNew(void);
  void strArrayAdd(UTStringArray *ar, char *str);
  void strArrayInsert(UTStringArray *ar, int i, char *str);
  void strArrayReset(UTStringArray *ar);
  void strArrayFree(UTStringArray *ar);
  char **strArray(UTStringArray *ar);
  uint32_t strArrayN(UTStringArray *ar);
  char *strArrayAt(UTStringArray *ar, int i);
  void strArraySort(UTStringArray *ar);
  char *strArrayStr(UTStringArray *ar, char *start, char *quote, char *delim, char *end);
  int strArrayEqual(UTStringArray *ar1, UTStringArray *ar2);
  int strArrayIndexOf(UTStringArray *ar, char *str);

  // obj array
  typedef struct _UTArray {
    void **objs;
    uint32_t n;
    uint32_t capacity;
  } UTArray;

  UTArray *UTArrayNew(void);
  void UTArrayAdd(UTArray *ar, void *obj);
  void UTArrayInsert(UTArray *ar, int i, void *obj);
  void UTArrayReset(UTArray *ar);
  void UTArrayFree(UTArray *ar);
  uint32_t UTArrayN(UTArray *ar);
  void *UTArrayAt(UTArray *ar, int i);

  // tokenizer
  char *parseNextTok(char **str, char *sep, int delim, char quot, int trim, char *buf, int buflen);

  // sleep
  void my_usleep(uint32_t microseconds);

  // calling execve()
  typedef int (*UTExecCB)(void *magic, char *line);
  int myExec(void *magic, char **cmd, UTExecCB lineCB, char *line, size_t lineLen, int *pstatus);

  // SFLAdaptor
  SFLAdaptor *adaptorNew(char *dev, u_char *macBytes, size_t userDataSize, uint32_t ifIndex);
  int adaptorEqual(SFLAdaptor *ad1, SFLAdaptor *ad2);
  void adaptorFree(SFLAdaptor *ad);
  
  // SFLAdaptorList
  SFLAdaptorList *adaptorListNew(void);
  void adaptorListReset(SFLAdaptorList *adList);
  void adaptorListFree(SFLAdaptorList *adList);
  void adaptorListMarkAll(SFLAdaptorList *adList);
  int adaptorListFreeMarked(SFLAdaptorList *adList);
  SFLAdaptor *adaptorListGet(SFLAdaptorList *adList, char *dev);
  SFLAdaptor *adaptorListGet_ifIndex(SFLAdaptorList *adList, uint32_t ifIndex);
  void adaptorListAdd(SFLAdaptorList *adList, SFLAdaptor *adaptor);
#define ADAPTORLIST_WALK(al, ad) for(uint32_t _ii = 0; _ii < (al)->num_adaptors; _ii++) if(((ad)=(al)->adaptors[_ii]))

  // file utils
  int truncateOpenFile(FILE *fptr);

  // SFLAddress utils
  int SFLAddress_equal(SFLAddress *addr1, SFLAddress *addr2);
  int SFLAddress_isLoopback(SFLAddress *addr);
  int SFLAddress_isSelfAssigned(SFLAddress *addr);
  int SFLAddress_isLinkLocal(SFLAddress *addr);
  int SFLAddress_isUniqueLocal(SFLAddress *addr);
  int SFLAddress_isMulticast(SFLAddress *addr);
  void SFLAddress_mask(SFLAddress *addr, SFLAddress *mask);  
  int SFLAddress_maskEqual(SFLAddress *addr, SFLAddress *mask, SFLAddress *compare);
  int SFLAddress_parseCIDR(char *str, SFLAddress *addr, SFLAddress *mask, uint32_t *maskBits);

  int isAllZero(u_char *buf, int len);
  int isZeroMAC(SFLMacAddress *mac);

  // UTHash
  typedef struct _UTHash {
    uint32_t f_offset;
    uint32_t f_len;
    uint32_t cap;
    uint32_t entries;
    void **bins;
  } UTHash;

  UTHash *UTHashNew(uint32_t f_offset, uint32_t f_len, int stringKey);
#define UTHASH_NEW(t,f,s) UTHashNew(offsetof(t, f), sizeof(((t *)0)->f), s)
  void UTHashFree(UTHash *oh);
  void UTHashAdd(UTHash *oh, void *obj, int overwrite_ok);
  void *UTHashGet(UTHash *oh, void *obj);
  int UTHashDel(UTHash *oh, void *obj);

#define UTHASH_WALK(oh, obj) for(uint32_t ii=0; ii<oh->cap; ii++) if(((obj)=(typeof(obj))oh->bins[ii]))
   
#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* UTIL_H */

