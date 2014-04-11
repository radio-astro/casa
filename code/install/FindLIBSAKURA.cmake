set(libsakura_install_prefix "$ENV{HOME}/workspace/libsakura/build/installed")
find_path(LIBSAKURA_INCLUDE_DIR libsakura/sakura.h PATHS ${libsakura_install_prefix}/include)
find_library(LIBSAKURA_LIBRARY NAMES sakura PATHS ${libsakura_install_prefix}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBSAKURA DEFAULT_MSG LIBSAKURA_LIBRARY LIBSAKURA_INCLUDE_DIR)

if(LIBSAKURA_FOUND)
  set(LIBSAKURA_LIBRARIES ${LIBSAKURA_LIBRARY})
  get_filename_component(LIBSAKURA_LIBRARY_PATH "${LIBSAKURA_LIBRARY}" PATH)
  SET(LIBSAKURA_EXE_LINKER_FLAGS "-L ${LIBSAKURA_LIBRARY_PATH} -Wl,-rpath,${LIBSAKURA_LIBRARY_PATH} -lsakura")
  SET(LIBSAKURA_INCLUDE_DIRS ${LIBSAKURA_INCLUDE_DIR})
else(LIBSAKURA_FOUND)
  set(LIBSAKURA_LIBRARIES)
  set(LIBSAKURA_INCLUDE_DIRS)
endif(LIBSAKURA_FOUND)

mark_as_advanced(LIBSAKURA_INCLUDE_DIR LIBSAKURA_LIBRARIES)
