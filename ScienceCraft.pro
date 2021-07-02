TEMPLATE = app
CONFIG += c++20
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_net -lGL -lGLEW -lpthread

SOURCES += \
        camera.cpp \
        chunk.cpp \
        client.cpp \
        dda.cpp \
        dist.cpp \
        gamewindow.cpp \
        main.cpp \
        mdlmanager.cpp \
        ray.cpp \
        server.cpp \
        shadermanager.cpp \
        texmanager.cpp

HEADERS += \
  camera.hpp \
  chunk.hpp \
  client.hpp \
  dda.hpp \
  dist.hpp \
  gamewindow.hpp \
  mdlmanager.hpp \
  ray.hpp \
  server.hpp \
  shadermanager.hpp \
  texmanager.hpp \
  PerlinNoise.hpp
