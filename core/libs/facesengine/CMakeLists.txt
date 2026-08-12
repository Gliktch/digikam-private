#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2024     , Michael Miller <michael underscore miller at msn dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# ---------------------------------------------------------------------------

kde_enable_exceptions()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/facedb

                    ${CMAKE_CURRENT_SOURCE_DIR}/detection
                    ${CMAKE_CURRENT_SOURCE_DIR}/detection/opencv-dnn

                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition
                    ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-dnn
)

# ---------------------------------------------------------------------------

set(facesengine_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/identity.cpp

                         ${CMAKE_CURRENT_SOURCE_DIR}/detection/facedetector.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/detection/opencv-dnn/opencvdnnfacedetector.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/detection/opencv-dnn/dnnfacedetectoryunet.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/detection/opencv-dnn/dnnfacedetectorbase.cpp

                         ${CMAKE_CURRENT_SOURCE_DIR}/recognition/recognitiontrainingupdatequeue.cpp
)

# Used by digikamcore
add_library(core_digikamfacesengine_obj
            OBJECT
            ${facesengine_LIB_SRCS}
)

target_compile_definitions(core_digikamfacesengine_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# For unit-tests
add_library(digikamfacesengine STATIC $<TARGET_OBJECTS:core_digikamfacesengine_obj>)

# ---------------------------------------------------------------------------

set(facesengine_database_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/recognition/faceclassifierbase.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/recognition/faceclassifier.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/recognition/identityprovider.cpp

                                  ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-dnn/dnnfaceextractorbase.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/recognition/opencv-dnn/dnnsfaceextractor.cpp

                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedbaccess.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedbbackend.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedboperationgroup.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedbschemaupdater.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedb.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedb_identity.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/facedb/facedb_dnn.cpp

                                  ${CMAKE_CURRENT_SOURCE_DIR}/widgets/facescansettings.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/widgets/facescanwidget.cpp
)

# Used by digikamgui
add_library(gui_digikamfacesenginedatabase_obj
            OBJECT
            ${facesengine_database_LIB_SRCS}
)

target_compile_definitions(gui_digikamfacesenginedatabase_obj
                           PRIVATE
                           digikamgui_EXPORTS
)

# For unit-tests
add_library(digikamfacesenginedatabase STATIC $<TARGET_OBJECTS:gui_digikamfacesenginedatabase_obj>)
