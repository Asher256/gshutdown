cmake_minimum_required(VERSION 2.8)

project(gshutdown)

add_executable(gshutdown src/action.c src/args.c src/bacon.c src/binreloc.c src/callback.c src/callback_main_window.c src/debug.c src/detectwm.c src/eggtrayicon.c src/gdm.c src/gui.c src/main.c src/misc.c src/pconfirm.c src/timer.c src/values.c)

install(TARGETS gshutdown DESTINATION bin)
