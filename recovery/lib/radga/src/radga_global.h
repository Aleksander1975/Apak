#ifndef ZN_RADGA_GLOBAL_H
#define ZN_RADGA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ZN_RADGA_LIBRARY)
#  define ZN_RADGASHARED_EXPORT Q_DECL_EXPORT
#else
#  define ZN_RADGASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ZN_RADGA_GLOBAL_H
