#ifndef MOXA_GLOBAL_H
#define MOXA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MOXA_LIBRARY)
#  define MOXA_EXPORT Q_DECL_EXPORT
#else
#  define MOXA_EXPORT Q_DECL_IMPORT
#endif

#endif // MOXA_GLOBAL_H
