#ifndef ZN_SKM_GLOBAL_H
#define ZN_SKM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ZN_SKM_LIBRARY)
#  define ZN_SKMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ZN_SKMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ZN_SKM_GLOBAL_H
