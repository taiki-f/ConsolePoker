#ifndef __DEFINE_H__
#define __DEFINE_H__

//
// Defines
//
typedef int BOOL;
#define TRUE  1
#define FALSE 0
#ifndef NULL
#   define NULL 0
#endif


//
// Macros
//
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

#endif  // __DEFINE_H__
