TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_LFLAGS += -pthread    # без неё не работают потоки STL

SOURCES += \
        main.cpp \
    logger.cpp

HEADERS += \
    logger.h \
    logger_base.h
