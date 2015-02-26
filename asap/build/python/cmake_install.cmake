# Install script for directory: /home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/linux")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/python/2.7/asap" TYPE PROGRAM FILES
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/__init__.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asapfit.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asapfitter.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asapgrid.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplinefind.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplot.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplotbase.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplotgui.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplotgui_gtk.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asaplotgui_qt4.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asapmath.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/asapplotter.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/compatibility.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/coordinate.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/customgui_base.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/customgui_qt4agg.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/customgui_tkagg.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/env.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/flagplotter.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/interactivemask.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/ipysupport.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/lagflagger.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/linecatalog.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/logging.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/opacity.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/parameters.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/plotter2.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/sbseparator.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/scantable.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/selector.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/simplelinefinder.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/edgemarker.py"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/svninfo.txt"
    "/home/warp/casa/dschieb/casa/trunk-gcmerge/trunk/asap/python/utils.py"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

