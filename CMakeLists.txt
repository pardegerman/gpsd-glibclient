cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(GIO gio-2.0)
pkg_check_modules(GPS libgps)

include_directories(${GLIB_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${GPS_INCLUDE_DIRS})
add_executable(gpsd-client src/main.c src/gpsd_source.c src/nxjson.c)
target_link_libraries(gpsd-client ${GLIB_LIBRARIES} ${GIO_LIBRARIES} ${GPS_LIBRARIES})
