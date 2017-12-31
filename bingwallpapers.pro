QT += qml quick widgets quickwidgets
CONFIG += c++11
LIBS += -lyaml-cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    bingio.cpp \
    bingio.cpp \
    main.cpp

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    bingio.h \
    include/yaml-cpp/contrib/anchordict.h \
    include/yaml-cpp/contrib/graphbuilder.h \
    include/yaml-cpp/node/detail/bool_type.h \
    include/yaml-cpp/node/detail/impl.h \
    include/yaml-cpp/node/detail/iterator.h \
    include/yaml-cpp/node/detail/iterator_fwd.h \
    include/yaml-cpp/node/detail/memory.h \
    include/yaml-cpp/node/detail/node.h \
    include/yaml-cpp/node/detail/node_data.h \
    include/yaml-cpp/node/detail/node_iterator.h \
    include/yaml-cpp/node/detail/node_ref.h \
    include/yaml-cpp/node/convert.h \
    include/yaml-cpp/node/emit.h \
    include/yaml-cpp/node/impl.h \
    include/yaml-cpp/node/iterator.h \
    include/yaml-cpp/node/node.h \
    include/yaml-cpp/node/parse.h \
    include/yaml-cpp/node/ptr.h \
    include/yaml-cpp/node/type.h \
    include/yaml-cpp/anchor.h \
    include/yaml-cpp/binary.h \
    include/yaml-cpp/dll.h \
    include/yaml-cpp/emitfromevents.h \
    include/yaml-cpp/emitter.h \
    include/yaml-cpp/emitterdef.h \
    include/yaml-cpp/emittermanip.h \
    include/yaml-cpp/emitterstyle.h \
    include/yaml-cpp/eventhandler.h \
    include/yaml-cpp/exceptions.h \
    include/yaml-cpp/mark.h \
    include/yaml-cpp/noncopyable.h \
    include/yaml-cpp/null.h \
    include/yaml-cpp/ostream_wrapper.h \
    include/yaml-cpp/parser.h \
    include/yaml-cpp/stlemitter.h \
    include/yaml-cpp/traits.h \
    include/yaml-cpp/yaml.h \
    bingio.h

DISTFILES += \
    include/yaml-cpp/lib/libyaml-cpp.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/include/yaml-cpp/lib/release/ -lyaml-cpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/include/yaml-cpp/lib/debug/ -lyaml-cpp
else:unix: LIBS += -L$$PWD/include/yaml-cpp/lib/ -lyaml-cpp

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/include/yaml-cpp/lib/release/libyaml-cpp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/include/yaml-cpp/lib/debug/libyaml-cpp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/include/yaml-cpp/lib/release/yaml-cpp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/include/yaml-cpp/lib/debug/yaml-cpp.lib
else:unix: PRE_TARGETDEPS += $$PWD/include/yaml-cpp/lib/libyaml-cpp.a
