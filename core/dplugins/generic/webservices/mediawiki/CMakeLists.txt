#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/backend
)

# --------------------------------------------------------------------------------

set(libmediawiki_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_imageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_iface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_login.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_logout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryimageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryimages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_querysiteinfogeneral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_generalinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_querysiteinfousergroups.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryrevision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_job.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_usergroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_revision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_page.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_protection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_upload.cpp
)

# For unit-tests and plugin
add_library(mediawikibackend STATIC ${libmediawiki_SRCS})

target_link_libraries(mediawikibackend
                      PRIVATE

                      digikamcore

                      KF${QT_VERSION_MAJOR}::CoreAddons
)

# --------------------------------------------------------------------------------

set(mediawikiplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikitalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MediaWiki
                           SOURCES ${mediawikiplugin_SRCS}
                           DEPENDS mediawikibackend
)

add_subdirectory(icons)
