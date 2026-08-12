#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

kde_enable_exceptions()

include_directories(${LCMS2_INCLUDE_DIR})

set(libdimg_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_bitsops.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_colors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_copy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_fileio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_metadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_props.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_qimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_qpixmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_scale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dimg_transform.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/color/dcolor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/color/dcolorcomposer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/history/dimagehistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/filteraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/historyimageid.cpp
)

set(libdimgfilters_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dimgbuiltinfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dimgthreadedfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dimgthreadedanalyser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dimgfiltermanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dimgfiltergenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/dpixelsaliasfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/filteractionfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/randomnumbergenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/raw/rawprocessingfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/raw/drawdecoding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/borderfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/bordersettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/bordercontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/texturecontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/texturefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/decorate/texturesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/film/filmfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/blurfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/blurfxfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/colorfxfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/colorfxsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/distortionfxfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/charcoalfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/embossfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/filmgrainfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/filmgrainsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/invertfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/oilpaintfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/fx/raindropfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/auto/autolevelsfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/auto/autoexpofilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/auto/equalizefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/auto/stretchfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/auto/normalizefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/cb/cbfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/cb/cbsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bcg/bcgfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bcg/bcgsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bcg/bcgcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/bwsepiafilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/bwsepiasettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/tonalityfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/infraredfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/mixerfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/bw/mixersettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hsl/hslfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hsl/hslsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hsl/hspreviewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/blackframeparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/blackframelistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/blackframelistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/blackframetooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/hotpixelprops.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/hotpixelfixer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/hotpixelsweights.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/hotpixelsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/hotpixels/hotpixelcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccprofile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccprofilesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/icctransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/icctransformfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p_desktop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p_wayland.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p_x11.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettingscontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/digikam-lcms.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lc/localcontrastfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lc/localcontrastsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lc/localcontrastcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/nr/nrfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/nr/nrestimate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/nr/nrsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp/sharpenfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp/unsharpmaskfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp/sharpsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/imagelevels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/levelsfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/imagehistogram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/histogrambox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/histogramwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/levels/histogrampainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/curvescontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/imagecurves.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/curvesfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/curvessettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/curveswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/curves/curvesbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/wb/wbcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/wb/wbfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/wb/wbsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/freerotationfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/freerotationsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/shearfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/autocrop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/greycstoration/greycstorationfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/greycstoration/greycstorationsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/antivignettingfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/antivignettingsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensdistortionfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensdistortionpixelaccess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/redeye/redeyecorrectionfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/redeye/redeyecorrectionsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/redeye/redeyecorrectioncontainer.cpp
)

if(APPLE)
    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/icc/iccsettings_p_macos.mm
    )
endif()

if(GLIB2_FOUND)

    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/contentawarefilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_carver_bias.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_carver.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_carver_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_carver_rigmask.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_cursor.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_energy.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_gradient.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_progress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_rwindow.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_vmap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/lqr_vmap_list.c
    )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filters/transform/lqr/)
    include_directories(${GLIB2_INCLUDE_DIR})

endif()

if(LensFun_FOUND)

    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensfunsettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensfunfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensfuncameraselector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/lens/lensfuniface.cpp
    )

    include_directories(${LENSFUN_INCLUDE_DIRS})

endif()

if(Eigen3_FOUND)

    set(libdimgfilters_SRCS
        ${libdimgfilters_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp/refocusfilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/filters/sharp/refocusmatrix.cpp
    )

endif()

set(libdimgloaders_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/loaders/dimgloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loaders/dimgloadersettings.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
)

if(X11_FOUND AND NOT Qt6_FOUND)

    include_directories($<TARGET_PROPERTY:Qt5::X11Extras,INTERFACE_INCLUDE_DIRECTORIES>)

endif()

if(X11_FOUND)

    include_directories(${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})

endif()

if(ENABLE_DBUS)

    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)

endif()

# Don't process automoc on LibCImg headers

file(GLOB_RECURSE cimg_headers ${CMAKE_CURRENT_SOURCE_DIR}/filters/greycstoration/cimg/*.h)

foreach(_file ${cimg_headers})

    set_property(SOURCE ${_file} PROPERTY SKIP_AUTOMOC ON)

endforeach()

# Used by digikamcore
add_library(core_dimg_obj OBJECT
            ${libdimgloaders_SRCS}
            ${libdimgfilters_SRCS}
            ${libdimg_SRCS}
)

target_compile_definitions(core_dimg_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

set_target_properties(core_dimg_obj
                      PROPERTIES
                      C_STANDARD 11                 # For liblqr code
                      C_STANDARD_REQUIRED YES
                      C_EXTENSIONS NO
)

if(Eigen3_FOUND)

    target_link_libraries(core_dimg_obj Eigen3::Eigen)

endif()
