#ifndef QT_FFMPEG_GLOBAL_H
#define QT_FFMPEG_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
#if defined(LIBQT_FFMPEG_LIBRARY)
#  define LIBQT_FFMPEG_EXPORT Q_DECL_EXPORT
#else
#  define LIBQT_FFMPEG_EXPORT Q_DECL_IMPORT
#endif
#else
#define LIBQT_FFMPEG_EXPORT
#endif

#endif // QT_FFMPEG_GLOBAL_H