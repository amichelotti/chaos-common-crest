cmake_minimum_required(VERSION 2.6)
IF( DEFINED ENV{CHAOS_BUNDLE} )
  include($ENV{CHAOS_BUNDLE}/tools/project_template/CMakeChaos.txt)
ENDIF()
# HTTP_LIB_DEBUG
# CREST_DEBUG
ADD_DEFINITIONS(-g -O2)
INCLUDE_DIRECTORIES(src)

SET(crest_src src/chaos_crest.c src/http_lib.c )
SET(crest_test_src test/crest_cu_test.c)
ADD_EXECUTABLE(crest_test ${crest_test_src})
ADD_LIBRARY(common-crest SHARED ${crest_src} )
TARGET_LINK_LIBRARIES(crest_test common-crest)
INSTALL_TARGETS(/lib  common-crest)

