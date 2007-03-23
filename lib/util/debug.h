#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define DBG(c)
#define DBG1(c)
#define DBG2(c)
#define ISDAEMON(i)
#define ASSERT(expr)

#else

extern void _assert(const char *, const char *, unsigned int);

#define DBG(c) c
#if DEBUG==2
#define DBG2(c) c
#else
#define DBG2(c)
#endif
#ifdef _MSC_VER
#define __STRING(x)	""
#endif

#define ASSERT(expr)	\
	if(expr){}	\
	else		\
	_assert(__STRING(expr), __FILE__, __LINE__)

#endif

#ifdef __cplusplus
}
#endif

#endif
