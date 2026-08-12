#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# enabling for everything although only required by DImg.
kde_enable_exceptions()

HEADER_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/utilities DK_LOCAL_INCLUDES_UTILS)

include_directories(${DK_LOCAL_INCLUDES}
                    ${DK_LOCAL_INCLUDES_UTILS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/
)

# -------------------------------------------------------------------------------
# Unit-tests common dependencies

set(_extra_deps ${_extra_deps} Qt${QT_VERSION_MAJOR}::WebEngineWidgets)

if(ENABLE_DBUS)

    set(_extra_deps ${_extra_deps} Qt${QT_VERSION_MAJOR}::DBus)

endif()

if(KF${QT_VERSION_MAJOR}Notifications_FOUND)

    set(_extra_deps ${_extra_deps} KF${QT_VERSION_MAJOR}::Notifications)

endif()

if(Gphoto2_FOUND)

    set(_extra_deps ${_extra_deps} ${GPHOTO2_LIBRARIES})

endif()

if(ImageMagick_Magick++_FOUND)

    set(_extra_deps ${_extra_deps} ${ImageMagick_LIBRARIES})

endif()

if(LensFun_FOUND)

    set(_extra_deps ${_extra_deps} ${LENSFUN_LIBRARIES})

endif()

if(Jasper_FOUND)

    set(_extra_deps ${_extra_deps} ${JASPER_LIBRARIES})

endif()

if(ENABLE_MEDIAPLAYER)

    set(_extra_deps ${_extra_deps} ${MEDIAPLAYER_LIBRARIES})

endif()

if(WIN32)

    set(_extra_deps ${_extra_deps} ws2_32)

endif()

if(NOT WIN32)

    # To link under Solaris (see bug #274484)
    set(_extra_deps ${_extra_deps} ${MATH_LIBRARY})

endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

    set(_extra_deps ${_extra_deps} ${KVM_LIBRARY})

endif()

if(APPLE)

    set(_extra_deps ${_extra_deps} /System/Library/Frameworks/AppKit.framework)

endif()

set(COMMON_TEST_LINK Qt${QT_VERSION_MAJOR}::Core
                     Qt${QT_VERSION_MAJOR}::Gui
                     Qt${QT_VERSION_MAJOR}::Xml
                     Qt${QT_VERSION_MAJOR}::Widgets
                     Qt${QT_VERSION_MAJOR}::Concurrent
                     Qt${QT_VERSION_MAJOR}::Test
                     Qt${QT_VERSION_MAJOR}::Sql

                     KF${QT_VERSION_MAJOR}::XmlGui
                     KF${QT_VERSION_MAJOR}::I18n
                     KF${QT_VERSION_MAJOR}::ConfigCore
                     KF${QT_VERSION_MAJOR}::CoreAddons
                     KF${QT_VERSION_MAJOR}::Service
                     KF${QT_VERSION_MAJOR}::Solid

                     ${OPENMP_LDFLAGS}
                     ${JPEG_LIBRARIES}
                     ${LCMS2_LIBRARIES}
                     ${EXPAT_LIBRARY}
                     ${CMAKE_THREAD_LIBS_INIT}

                     opencv_core
                     opencv_objdetect
                     opencv_imgproc
                     opencv_imgcodecs
                     opencv_dnn
                     opencv_flann

                     ${_extra_deps}
)

# -------------------------------------------------------------------------------
# Common static library for uint-tests based on ModelTest

include_directories(modeltest)

add_library(libmodeltest STATIC
            modeltest/modeltest.cpp
)

target_link_libraries(libmodeltest
                      PRIVATE
                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Test
)

# -------------------------------------------------------------------------------
# Unit-tests and CLI tools

add_subdirectory(advancedrename)
# add_subdirectory(autotagassignment)
add_subdirectory(albummodel)                    # Unit-test based on ModelTest
add_subdirectory(database)                      # Some unit-tests based on ModelTest
add_subdirectory(dimg)
add_subdirectory(dplugins)
add_subdirectory(facesengine)
add_subdirectory(fileio)
add_subdirectory(filters)
add_subdirectory(imgqsort)
add_subdirectory(iojobs)
add_subdirectory(itemview)
add_subdirectory(mediaserver)
add_subdirectory(mjpegstream)
add_subdirectory(mediawiki)
add_subdirectory(metadataengine)
add_subdirectory(miscs)
add_subdirectory(multithreading)
add_subdirectory(ocrtextconverter)
add_subdirectory(rawengine)
add_subdirectory(timestampupdate)
add_subdirectory(webservices)
add_subdirectory(widgets)
add_subdirectory(video)


if(JXL_FOUND)

    # DNG support

    add_subdirectory(dngwriter)

endif()

if(ENABLE_GEOLOCATION)

    add_subdirectory(geolocation)               # Some unit-tests based on ModelTest

endif()

if(FLEX_FOUND AND BISON_FOUND AND KF${QT_VERSION_MAJOR}ThreadWeaver_FOUND)

    add_subdirectory(panorama)

endif()
