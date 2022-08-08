#ifndef ZN_K1_GLOBAL_H
#define ZN_K1_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ZN_K1_LIBRARY)
#  define ZN_K1SHARED_EXPORT Q_DECL_EXPORT
#else
#  define ZN_K1SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ZN_K1_GLOBAL_H
