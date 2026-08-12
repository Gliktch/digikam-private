#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# digiKam core object library
#

set(DIGIKAMCORE_OBJECTS

            ${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_debug.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.cpp

            $<TARGET_OBJECTS:core_dimg_obj>
            $<TARGET_OBJECTS:core_dragdrop_obj>
            $<TARGET_OBJECTS:core_dmetadata_obj>
            $<TARGET_OBJECTS:core_onlineversion_obj>
            $<TARGET_OBJECTS:core_jpegutils_obj>
            $<TARGET_OBJECTS:core_progressmanager_obj>
            $<TARGET_OBJECTS:core_threadimageio_obj>
            $<TARGET_OBJECTS:core_pgfutils_obj>
            $<TARGET_OBJECTS:core_dthread_obj>
            $<TARGET_OBJECTS:core_networkmanager_obj>
            $<TARGET_OBJECTS:core_versionmanager_obj>
            $<TARGET_OBJECTS:core_libraw_obj>
            $<TARGET_OBJECTS:core_rawengine_obj>
            $<TARGET_OBJECTS:core_dpluginsinterface_obj>
            $<TARGET_OBJECTS:core_libwso2_obj>
            $<TARGET_OBJECTS:core_dnnmodelmanager_obj>
            $<TARGET_OBJECTS:core_mlpipeline_obj>
            $<TARGET_OBJECTS:core_qtopencvimg_obj>
            $<TARGET_OBJECTS:core_autorotator_obj>

            # widgets
            $<TARGET_OBJECTS:core_digikamwidgets_obj>
            $<TARGET_OBJECTS:core_digikamdialogs_obj>
            $<TARGET_OBJECTS:core_itemproperties_obj>
            $<TARGET_OBJECTS:core_digikamgenericmodels_obj>
            $<TARGET_OBJECTS:core_notificationmanager_obj>

            # utilities
            $<TARGET_OBJECTS:core_setupcommon_obj>
            $<TARGET_OBJECTS:core_imageeditor_obj>
            $<TARGET_OBJECTS:core_libtransitionmngr_obj>
            $<TARGET_OBJECTS:core_timeadjust_obj>

            $<TARGET_OBJECTS:core_digikamdatabase_obj>
            $<TARGET_OBJECTS:core_digikamfacesengine_obj>
            $<TARGET_OBJECTS:core_videotoolscommon_obj>
)

if(JXL_FOUND)

    # DNG support

    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:core_libmd5_obj>
        $<TARGET_OBJECTS:core_libxmp_obj>
        $<TARGET_OBJECTS:core_libdng_obj>
        $<TARGET_OBJECTS:core_dngwriter_obj>
    )

endif()

if(ENABLE_MEDIAPLAYER)

    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:core_videotools_obj>
    )

    if (NOT ENABLE_QTMULTIMEDIA)

        set(DIGIKAMCORE_OBJECTS
            ${DIGIKAMCORE_OBJECTS}
            $<TARGET_OBJECTS:core_qtavplayer_obj>
        )

    endif()

endif()

if(ENABLE_GEOLOCATION)

    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:core_geoiface_obj>
        $<TARGET_OBJECTS:core_geomapwrapper_obj>
        $<TARGET_OBJECTS:core_marble_obj>
    )

endif()

if(KF${QT_VERSION_MAJOR}FileMetaData_FOUND)

    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:core_baloowrap_obj>
    )

endif()

if(KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND OR KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND)

    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:core_akonadiiface_obj>
    )

endif()

add_library(digikamcore
            SHARED
            ${DIGIKAMCORE_OBJECTS}
)

add_dependencies(digikamcore digikam-gitversion)
add_dependencies(digikamcore digikam-builddate)

set_target_properties(digikamcore PROPERTIES
                      VERSION ${DIGIKAM_VERSION_SHORT}
                      SOVERSION ${DIGIKAM_VERSION_SHORT}
)

target_compile_definitions(digikamcore
                           PRIVATE
                           digikamcore_EXPORTS
)

target_include_directories(digikamcore
                           PRIVATE
                           ${DIGIKAM_TARGET_INCLUDES}
)

# All codes from this target are exported with digikam_core_export.h header and DIGIKAM_EXPORT macro.
generate_export_header(digikamcore
                       BASE_NAME digikam
                       EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_core_export.h"
)

# NOTE: all this target dependencies must be private and not exported
# to prevent inherited dependencies on external plugins.

target_link_libraries(digikamcore

                      PRIVATE

                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Gui
                      Qt${QT_VERSION_MAJOR}::Xml
                      Qt${QT_VERSION_MAJOR}::Widgets
                      Qt${QT_VERSION_MAJOR}::Sql
                      Qt${QT_VERSION_MAJOR}::PrintSupport
                      Qt${QT_VERSION_MAJOR}::Concurrent
                      Qt${QT_VERSION_MAJOR}::Svg
                      Qt${QT_VERSION_MAJOR}::WebEngineWidgets

                      KF${QT_VERSION_MAJOR}::Solid
                      KF${QT_VERSION_MAJOR}::WindowSystem
                      KF${QT_VERSION_MAJOR}::ConfigGui
                      KF${QT_VERSION_MAJOR}::XmlGui
                      KF${QT_VERSION_MAJOR}::I18n
                      KF${QT_VERSION_MAJOR}::Service
                      KF${QT_VERSION_MAJOR}::CoreAddons

                      # Required by CImg which use pthread internally.

                      ${CMAKE_THREAD_LIBS_INIT}
                      ${EXPAT_LIBRARY}

                      ${LCMS2_LIBRARIES} # filters

                      ${TIFF_LIBRARIES}
                      PNG::PNG
                      ${JPEG_LIBRARIES}

                      ${OPENMP_LDFLAGS}

                      opencv_core
                      opencv_objdetect
                      opencv_imgproc
                      opencv_imgcodecs
                      opencv_dnn
                      opencv_ml
                      opencv_flann
)

if(JXL_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${JXL_LIBRARIES}
    )

endif()

if(LibExiv2_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${LibExiv2_LIBRARIES}
    )

else()

    target_link_libraries(digikamcore
                          PRIVATE
                          exiv2lib
    )

endif()

if(Qt6_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          Qt${QT_VERSION_MAJOR}::StateMachine
                          Qt${QT_VERSION_MAJOR}::SvgWidgets
    )

endif()


if(ENABLE_DBUS)

    target_link_libraries(digikamcore
                          PRIVATE
                          Qt${QT_VERSION_MAJOR}::DBus
    )

endif()

if(ENABLE_MEDIAPLAYER)

    target_link_libraries(digikamcore
                          PRIVATE
                          Qt${QT_VERSION_MAJOR}::Multimedia
                          Qt${QT_VERSION_MAJOR}::MultimediaWidgets
                          ${MEDIAPLAYER_LIBRARIES}
    )

endif()

if(KF${QT_VERSION_MAJOR}IconThemes_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::IconThemes
    )

    if(Qt6_FOUND)

        target_link_libraries(digikamcore
                              PRIVATE
                              KF${QT_VERSION_MAJOR}::IconWidgets
        )

    endif()

endif()

if(KF${QT_VERSION_MAJOR}KIO_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::KIOCore
                          KF${QT_VERSION_MAJOR}::KIOWidgets
    )

endif()

if(KF${QT_VERSION_MAJOR}Notifications_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::Notifications
    )

endif()

if(KF${QT_VERSION_MAJOR}NotifyConfig_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::NotifyConfig
    )

endif()

if(KF${QT_VERSION_MAJOR}Sonnet_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::SonnetCore
                          KF${QT_VERSION_MAJOR}::SonnetUi
    )

endif()

if(X11_FOUND)

    if(Qt6_FOUND)

        target_link_libraries(digikamcore
                              PRIVATE
                              ${X11_LIBRARIES}
        )

    else()


        target_link_libraries(digikamcore
                              PRIVATE
                              Qt5::X11Extras
                              ${X11_LIBRARIES}
        )

    endif()

endif()

if(Jasper_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${JASPER_LIBRARIES}
    )

endif()

# LibLqr-1 library rules for content-aware filter
if(GLIB2_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${GLIB2_LIBRARIES}
    )

endif()

# For HEIF file format support
if(HEIF_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          Libheif::Libheif
    )

endif()

if(X265_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${X265_LIBRARIES}
    )

endif()

if(LensFun_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${LENSFUN_LIBRARIES}
    )

endif()

if(Magick_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${Magick_LIBRARIES}
    )

endif()

# for nrfilter
if(OpenCV_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${OpenCV_LIBRARIES}
    )

endif()

if(KF${QT_VERSION_MAJOR}FileMetaData_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KF${QT_VERSION_MAJOR}::FileMetaData
    )

endif()

if(KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KPim${QT_VERSION_MAJOR}::AkonadiContact
    )

elseif(KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND)

    target_link_libraries(digikamcore
                          PRIVATE
                          KPim${QT_VERSION_MAJOR}::AkonadiContactCore
    )

endif()

if(APPLE)

    target_link_libraries(digikamcore
                          PRIVATE
                          "-framework AppKit"
                          "-framework IOKit"
                          "-framework CoreGraphics"
                          "-framework ColorSync"
    )

endif()

if(WIN32)

    target_link_libraries(digikamcore
                          PRIVATE
                          # Defined in RulesWindows.cmake
                          ${WSOCK32_LIBRARY}
                          ${WS2_32_LIBRARY}
                          ${BCRYPT_LIBRARY}
                          ${NETAPI32_LIBRARY}
                          ${USEENV_LIBRARY}
                          ${PSAPI_LIBRARY}
                          ${MSCMS_LIBRARY}
    )

endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

    target_link_libraries(digikamcore
                          PRIVATE
                          ${KVM_LIBRARY}
    )

endif()

# Share the install include directory for the 3rdparty plugins
target_include_directories(digikamcore INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/digikam>")

### Install Rules ###############################################################################################################

install(TARGETS digikamcore EXPORT DigikamCoreConfig ${INSTALL_TARGETS_DEFAULT_ARGS})

install(EXPORT DigikamCoreConfig DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DigikamCore" NAMESPACE Digikam::)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/DigikamCoreConfigVersion.cmake
                                 VERSION ${DIGIKAM_VERSION_SHORT}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DigikamCoreConfigVersion.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DigikamCore")

# install debug symbols

if(MSVC)
    install(FILES "$<TARGET_PDB_FILE:digikamcore>" DESTINATION "${CMAKE_INSTALL_BINDIR}" CONFIGURATIONS Debug RelWithDebInfo)
endif()

if(APPLE)
    install(FILES "$<TARGET_FILE:digikamcore>.dSYM" DESTINATION "${CMAKE_INSTALL_LIBDIR}" CONFIGURATIONS Debug RelWithDebInfo)
endif()
