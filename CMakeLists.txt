cmake_minimum_required(VERSION 2.9)
project(common_crest)
# HTTP_LIB_DEBUG
# CREST_DEBUG
ADD_DEFINITIONS(-g -O2)
INCLUDE_DIRECTORIES(.)
IF (CMAKE_HOST_APPLE)
   ADD_DEFINITIONS(-DOSX)
   MESSAGE(STATUS "OSX variant")
ENDIF()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(crest_src chaos_crest.c http_lib.c mongoose.c encode.c)
SET(crest_test_src test/crest_cu_test.c)
SET(crest_test_ui_src test/chaos_crest_ui_test.c)
ADD_EXECUTABLE(crest_test ${crest_test_src})
ADD_EXECUTABLE(het_acquire het/src/hetAcquire.c het/src/hetDriver.c)

ADD_EXECUTABLE(chaos_crest_ui_test ${crest_test_ui_src})
# IF(BUILD_FORCE_STATIC)
ADD_LIBRARY(common-crest STATIC ${crest_src} )
#ELSE()
#ADD_LIBRARY(common-crest SHARED ${crest_src} )
#ENDIF()
TARGET_LINK_LIBRARIES(crest_test common-crest pthread)
TARGET_LINK_LIBRARIES(chaos_crest_ui_test common-crest pthread)
TARGET_LINK_LIBRARIES(het_acquire common-crest pthread)

INSTALL(FILES chaos_crest.h DESTINATION include/common/crest)
#INSTALL_TARGETS(/lib  common-crest)
INSTALL(TARGETS common-crest
        DESTINATION "lib"
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
#INSTALL_TARGETS(/bin  crest_test)
INSTALL(TARGETS crest_test
        DESTINATION "bin"
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
#INSTALL_TARGETS(/bin  chaos_crest_ui_test)
INSTALL(TARGETS chaos_crest_ui_test
        DESTINATION "bin"
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

