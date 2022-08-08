#ifndef UPZ_GLOBAL_H
#define UPZ_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UPZ_LIBRARY)
#  define UPZ_EXPORT Q_DECL_EXPORT
#else
#  define UPZ_EXPORT Q_DECL_IMPORT
#endif

#endif // UPZ_GLOBAL_H
