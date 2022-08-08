#ifndef UPS_GLOBAL_H
#define UPS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UPS_LIBRARY)
#  define UPS_EXPORT Q_DECL_EXPORT
#else
#  define UPS_EXPORT Q_DECL_IMPORT
#endif

#endif // UPS_GLOBAL_H
