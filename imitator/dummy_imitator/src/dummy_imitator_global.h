﻿#ifndef DUMMY_IMITATOR_GLOBAL_H
#define DUMMY_IMITATOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DUMMY_IMITATOR_LIBRARY)
#  define DUMMY_IMITATOR_EXPORT Q_DECL_EXPORT
#else
#  define DUMMY_IMITATOR_EXPORT Q_DECL_IMPORT
#endif

#endif // DUMMY_IMITATOR_GLOBAL_H
