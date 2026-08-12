#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(digikamhelperdir ${KDE_INSTALL_FULL_DATADIR}/digikam/utils)

#configure_file(digikam-download.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop @ONLY)
#configure_file(digikam-gphoto2-camera.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop @ONLY)
#configure_file(digikam-mount-and-download.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop @ONLY)

configure_file(digikam-opencamera.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop @ONLY)

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop
#              ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop
#              ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop
#        DESTINATION ${KDE_INSTALL_FULL_KSERVICESDIR}/ServiceMenus)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop DESTINATION ${KDE_INSTALL_FULL_DATADIR}/solid/actions)

if(NOT WIN32)
    install(PROGRAMS digikam-camera DESTINATION ${digikamhelperdir})
endif()
