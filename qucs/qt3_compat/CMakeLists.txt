# qucs/qt3_compat library

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

#INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) -I$(top_srcdir)/qucs

SET(QT3_COMPAT_HDRS
q3frame.h
q3gdict.h
q3ptrcollection.h
q3ptrdict.h
q3scrollview.h
qt_compat.h
q3ptrlist.h
q3glist.h
q3gvector.h
#q3valuelist.h
)

SET(QT3_COMPAT_SRCS
q3frame.cpp
q3gdict.cpp
q3gvector.cpp
q3glist.cpp
q3scrollview.cpp
)

SET(QT3_COMPAT_MOC_HDRS
q3scrollview.h
q3frame.h
)


QT6_WRAP_CPP( QT3_COMPAT_SRCS ${QT3_COMPAT_MOC_HDRS} )

ADD_LIBRARY(qt3_compat STATIC ${QT3_COMPAT_HDRS} ${QT3_COMPAT_SRCS})
