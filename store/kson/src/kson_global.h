#ifndef KSON_GLOBAL_H
#define KSON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(KSON_LIBRARY)
#  define KSON_EXPORT Q_DECL_EXPORT
#else
#  define KSON_EXPORT Q_DECL_IMPORT
#endif

#endif // KSON_GLOBAL_H
