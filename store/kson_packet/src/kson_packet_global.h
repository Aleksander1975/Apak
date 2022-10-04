#ifndef KSON_PACKET_GLOBAL_H
#define KSON_PACKET_GLOBAL_H

//#include <QtCore/qglobal.h>

#if defined(KSON_PACKET_LIBRARY)
#  define KSON_PACKET_EXPORT Q_DECL_EXPORT
#else
#  define KSON_PACKET_EXPORT Q_DECL_IMPORT
#endif

#endif // KSON_PACKET_GLOBAL_H
