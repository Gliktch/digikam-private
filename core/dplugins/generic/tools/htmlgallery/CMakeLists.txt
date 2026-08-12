#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Service,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/generator
                    ${CMAKE_CURRENT_SOURCE_DIR}/parameters
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

if(NOT APPLE)

    include_directories(${LIBXSLT_INCLUDE_DIR}
                        ${LIBXML2_INCLUDE_DIR}
    )

    add_definitions(${LIBXSLT_DEFINITIONS}
                    ${LIBXML2_DEFINITIONS}
    )

endif()

set(htmlgalleryplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/abstractthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/intthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/listthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/captionthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/stringthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/colorthemeparameter.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/invisiblebuttongroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlthemepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmloutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlimagesettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlparameterspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlwizard.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryxmlutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerynamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelementfunctor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerytheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerygenerator.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/htmlgalleryplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    HtmlGallery
                           SOURCES ${htmlgalleryplugin_SRCS}
                           DEPENDS Qt${QT_VERSION_MAJOR}::Concurrent
                                   ${LIBXSLT_EXSLT_LIBRARIES}
                                   ${LIBXSLT_LIBRARIES}
                                   ${LIBXML2_LIBRARIES}
)

install(DIRECTORY themes DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/)
