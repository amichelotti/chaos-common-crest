cmake_minimum_required(VERSION 2.6)
IF( DEFINED ENV{CHAOS_BUNDLE} )
  include($ENV{CHAOS_BUNDLE}/tools/project_template/CMakeChaos.txt)
ENDIF()

INCLUDE_DIRECTORIES(src)

SET(crest_src src/chaos_crest.c src/http_lib.c test/crest_cu_test.c)

ADD_LIBRARY(common_crest SHARED ${crest_src} )
INSTALL_TARGETS(/lib  common_crest)

