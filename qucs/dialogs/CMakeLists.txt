# qucs/dialogs library

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} ) # ui_*.h, out of tree

#INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) -I$(top_srcdir)/qucs

SET(DIALOGS_HDRS
aboutdialog.h
changedialog.h
digisettingsdialog.h
exportdialog.h
importdialog.h
labeldialog.h
librarydialog.h
loaddialog.h
matchdialog.h
newprojdialog.h
qucssettingsdialog.h
savedialog.h
searchdialog.h
settingsdialog.h
simmessage.h
sweepdialog.h
vasettingsdialog.h
textboxdialog.h
displaydialog.h
tuner.h
fillfromspicedialog.h
)



SET(DIALOGS_SRCS
changedialog.cpp vasettingsdialog.cpp textboxdialog.cpp
digisettingsdialog.cpp		qucssettingsdialog.cpp
importdialog.cpp		savedialog.cpp labeldialog.cpp
searchdialog.cpp     librarydialog.cpp settingsdialog.cpp
matchdialog.cpp			simmessage.cpp newprojdialog.cpp
sweepdialog.cpp			exportdialog.cpp loaddialog.cpp
aboutdialog.cpp
displaydialog.cpp
tuner.cpp
fillfromspicedialog.cpp
)

SET(DIALOGS_MOC_HDRS
aboutdialog.h
changedialog.h
digisettingsdialog.h
exportdialog.h
importdialog.h
labeldialog.h
librarydialog.h
loaddialog.h
matchdialog.h
newprojdialog.h
qucssettingsdialog.h
savedialog.h
searchdialog.h
settingsdialog.h
simmessage.h
sweepdialog.h
sweepdialog.h
vasettingsdialog.h
textboxdialog.h
displaydialog.h
tuner.h
fillfromspicedialog.h
)

SET(DIALOGS_UIC_HDRS
searchdialog.ui
)


QT6_WRAP_CPP( DIALOGS_MOC_SRCS ${DIALOGS_MOC_HDRS} )
QT6_WRAP_UI( DIALOGS_UIC_SRCS ${DIALOGS_UIC_HDRS} )

ADD_LIBRARY(dialogs STATIC ${DIALOGS_HDRS} ${DIALOGS_SRCS} ${DIALOGS_MOC_SRCS} ${DIALOGS_UIC_SRCS})
