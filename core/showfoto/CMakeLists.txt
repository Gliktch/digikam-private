#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# To fill MacOS and Windows bundles metadata

set(BUNDLE_APP_NAME_STRING          "showfoto")
set(BUNDLE_APP_DESCRIPTION_STRING   "Stand alone Photo Editor based on digiKam Image Editor")
set(BUNDLE_LEGAL_COPYRIGHT_STRING   "GNU Public License V2")
set(BUNDLE_COMMENT_STRING           "Free and open source software to edit photo")
set(BUNDLE_LONG_VERSION_STRING      ${DIGIKAM_VERSION_STRING})
set(BUNDLE_SHORT_VERSION_STRING     ${DIGIKAM_VERSION_SHORT})
set(BUNDLE_VERSION_STRING           ${DIGIKAM_VERSION_STRING})

# Showfoto executable

set(showfoto_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetuptooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupmisc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupmetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupplugins.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetup.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoiteminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoitemmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotothumbnailmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotofiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotodragdrophandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotokineticscroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoitemsortsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotocoordinatesoverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoitemviewdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotodelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfototooltipfiller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotocategorizedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotothumbnailbar.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewsidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewundo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewtooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewbookmarks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewbookmarkdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewbookmarklist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/folderview/showfotofolderviewbookmarkitem.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewsidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewtooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewfavorites.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewfavoriteitemdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewfavoritelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackview/showfotostackviewfavoriteitem.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfotosettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfotoinfoiface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_open.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_iofiles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_import.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto_thumbbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
)

# Set the application icon on the application

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/core/data/icons/apps/*-apps-showfoto.png")

if(WIN32)

    # Build the main implementation into a DLL to be called by a stub EXE.
    # This is a work around "command line is too long" issue on Windows.
    # see https://stackoverflow.com/questions/43184251/cmake-command-line-too-long-windows

    add_library(showfoto SHARED ${showfoto_SRCS})
    set_target_properties(showfoto PROPERTIES PREFIX "")

elseif(APPLE)

    ecm_add_app_icon(showfoto_SRCS ICONS ${ICONS_SRCS})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/ShowfotoInfo.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
    add_executable(showfoto ${showfoto_SRCS})
    set_target_properties(showfoto PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

else()

    ecm_add_app_icon(showfoto_SRCS ICONS ${ICONS_SRCS})
    add_executable(showfoto ${showfoto_SRCS})

endif()

HEADER_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/utilities DK_LOCAL_INCLUDES_UTILS)

target_include_directories(showfoto

                           # We need to include the setup dir for showfoto BEFORE the other include dirs,
                           # otherwise the one from the "utilities" dir will be used

                           BEFORE
                           PRIVATE

                           ${CMAKE_CURRENT_SOURCE_DIR}/folderview
                           ${CMAKE_CURRENT_SOURCE_DIR}/stackview
                           ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar
                           ${CMAKE_CURRENT_SOURCE_DIR}/setup
                           ${CMAKE_CURRENT_SOURCE_DIR}/main
)

target_include_directories(showfoto

                           AFTER
                           PRIVATE

                           $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>

                           ${CMAKE_SOURCE_DIR}/core/app/dragdrop
                           ${CMAKE_SOURCE_DIR}/core/app/items/overlays
                           ${CMAKE_SOURCE_DIR}/core/app/views/sidebar

                           ${DK_LOCAL_INCLUDES}
                           ${DK_LOCAL_INCLUDES_UTILS}
)

if(KF${QT_VERSION_MAJOR}IconThemes_FOUND)

    target_include_directories(showfoto
                               AFTER
                               PRIVATE
                               $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::IconThemes,INTERFACE_INCLUDE_DIRECTORIES>
    )

    if (Qt6_FOUND)

        target_include_directories(showfoto
                                   AFTER
                                   PRIVATE
                                   $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::IconWidgets,INTERFACE_INCLUDE_DIRECTORIES>
        )

    endif()

endif()

add_dependencies(showfoto digikam-gitversion)
add_dependencies(showfoto digikam-builddate)

target_link_libraries(showfoto

                      PUBLIC

                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Gui
                      Qt${QT_VERSION_MAJOR}::Xml
                      Qt${QT_VERSION_MAJOR}::Widgets

                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::I18n
                      KF${QT_VERSION_MAJOR}::ConfigCore
                      KF${QT_VERSION_MAJOR}::Service
                      KF${QT_VERSION_MAJOR}::CoreAddons

                      digikamcore
)

if(KF${QT_VERSION_MAJOR}Notifications_FOUND)

    target_link_libraries(showfoto
                          PUBLIC
                          KF${QT_VERSION_MAJOR}::Notifications
    )

endif()

if(KF${QT_VERSION_MAJOR}NotifyConfig_FOUND)

    target_link_libraries(showfoto
                          PUBLIC
                          KF${QT_VERSION_MAJOR}::NotifyConfig
    )

endif()

if(KF${QT_VERSION_MAJOR}IconThemes_FOUND)

    target_link_libraries(showfoto
                          PUBLIC
                          KF${QT_VERSION_MAJOR}::IconThemes
    )

    if (Qt6_FOUND)

        target_link_libraries(showfoto
                              PUBLIC
                              KF${QT_VERSION_MAJOR}::IconWidgets
        )

    endif()

endif()

if(ImageMagick_Magick++_FOUND)

    target_link_libraries(showfoto
                          PUBLIC
                          ${ImageMagick_LIBRARIES}
    )

endif()

if(NOT WIN32)

    # To link under Solaris (see bug #274484)
    target_link_libraries(showfoto
                          PUBLIC
                          ${MATH_LIBRARY}
    )

endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

    target_link_libraries(showfoto
                          PUBLIC
                          ${KVM_LIBRARY}
    )

endif()

if(APPLE)

    target_link_libraries(showfoto
                          PUBLIC
                          /System/Library/Frameworks/AppKit.framework
    )

endif()

if(Qt6_FOUND)
    install(TARGETS showfoto DESTINATION ${KDE_INSTALL_BINDIR})
else()
    install(TARGETS showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/main/org.kde.showfoto.desktop     DESTINATION ${KDE_INSTALL_FULL_APPDIR})
install(FILES    ${CMAKE_CURRENT_SOURCE_DIR}/main/org.kde.showfoto.appdata.xml DESTINATION ${KDE_INSTALL_FULL_METAINFODIR})
install(FILES    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfotoui5.rc               DESTINATION ${KDE_INSTALL_FULL_KXMLGUIDIR}/showfoto)

if(APPLE)
    install(FILES "$<TARGET_FILE:showfoto>.dSYM" DESTINATION "${CMAKE_INSTALL_BINDIR}" CONFIGURATIONS Debug RelWithDebInfo)
endif()

if(WIN32)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/versioninfo.rc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)

    set(showfoto_windows_stub_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/main/windows_stub_main.cpp)

    ecm_add_app_icon(showfoto_windows_stub_SRCS ICONS ${ICONS_SRCS})

    add_executable(showfoto_windows_stub_exe
                   ${showfoto_windows_stub_SRCS}
                   ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc
    )

    target_link_libraries(showfoto_windows_stub_exe
                          PRIVATE
                          showfoto)

    if(NOT Qt6_FOUND)

        target_link_libraries(showfoto_windows_stub_exe
                              PRIVATE
                              Qt${QT_VERSION_MAJOR}::WinMain)

    endif()

    set_target_properties(showfoto_windows_stub_exe PROPERTIES OUTPUT_NAME "showfoto")
    target_include_directories(showfoto_windows_stub_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main)

    install(TARGETS showfoto_windows_stub_exe ${INSTALL_TARGETS_DEFAULT_ARGS})

endif()

if(WIN32)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/showfoto.exe.manifest.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/showfoto.exe.manifest)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/showfoto.exe.manifest DESTINATION ${KDE_INSTALL_BINDIR})

endif()
