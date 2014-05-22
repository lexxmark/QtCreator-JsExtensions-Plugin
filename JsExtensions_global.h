#ifndef JEP_GLOBAL_H
#define JEP_GLOBAL_H

#include <QtGlobal>

#if defined(JEP_LIBRARY)
#  define JEP_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define JEP_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // JEP_GLOBAL_H

