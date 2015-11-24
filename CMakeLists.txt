cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(GLADE REQUIRED libglade-2.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})
include_directories(${GLADE_INCLUDE_DIRS})
link_directories(${GLADE_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK2_CFLAGS_OTHER})
add_definitions(${GLADE_CFLAGS_OTHER})

project(gshutdown)

add_executable(gshutdown src/action.c src/args.c src/bacon.c src/binreloc.c src/callback.c src/callback_main_window.c src/debug.c src/detectwm.c src/eggtrayicon.c src/gdm.c src/gui.c src/main.c src/misc.c src/pconfirm.c src/timer.c src/values.c)

install(TARGETS gshutdown DESTINATION bin)
