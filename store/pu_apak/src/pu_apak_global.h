#ifndef PU_APAK_GLOBAL_H
#define PU_APAK_GLOBAL_H

//#include <QtCore/qglobal.h>

#if defined(PU_APAK_LIBRARY)
#  define PU_APAK_EXPORT Q_DECL_EXPORT
#else
#  define PU_APAK_EXPORT Q_DECL_IMPORT
#endif

#endif // PU_APAK_H
