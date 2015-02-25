cmake_minimum_required(VERSION 2.6)
IF( DEFINED ENV{CHAOS_BUNDLE} )
  include($ENV{CHAOS_BUNDLE}/tools/project_template/CMakeChaos.txt)
ENDIF()

INCLUDE_DIRECTORIES(vme vmewrap)
LINK_DIRECTORIES(./lib)
    set (CMAKE_C_FLAGS "-m32")
    set (CMAKE_CXX_FLAGS "-m32")
    set (CMAKE_LINK_FLAGS "-m32")

SET(sis_src sis/sis3800_drv.c)

SET(vmewrap_src vmewrap/vmewrap.c)
SET(caen_src caen/src/caen965_drv.c caen/src/caen513_drv.c caen/src/caen792_drv.c )
SET(caen960_test_src test/test_qdc_caen.c )
SET(test_btf_daq_src test/test_btf_daq.c )


ADD_LIBRARY(common_vme SHARED ${caen_src} ${vmewrap_src} ${sis_src})
ADD_EXECUTABLE(caen_qdc_test ${caen960_test_src})
ADD_EXECUTABLE(test_btf_daq ${test_btf_daq_src})

TARGET_LINK_LIBRARIES(caen_qdc_test common_vme vme)
TARGET_LINK_LIBRARIES(test_btf_daq common_vme vme)

INSTALL_TARGETS(/lib  common_vme)
INSTALL_TARGETS(/bin caen_qdc_test)
INSTALL_TARGETS(/bin test_btf_daq)
