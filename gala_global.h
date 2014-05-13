#ifndef GALA_GLOBAL_H
#define GALA_GLOBAL_H

#include <QtGlobal>

#if defined(GALA_LIBRARY)
#  define GALASHARED_EXPORT Q_DECL_EXPORT
#else
#  define GALASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GALA_GLOBAL_H

