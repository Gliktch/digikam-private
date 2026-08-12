#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

kde_enable_exceptions()

# TODO: How to extract exiv2lib definitions properties with cmake import interface?
#add_definitions($<TARGET_PROPERTY:exiv2lib,INTERFACE_COMPILE_DEFINITIONS>)

if(WIN32)
    add_definitions(-DEXV_HAVE_DLL)
endif()

# Exiv2 version < 0.28 use auto_ptr instead unique_ptr which is deprecated.

if("${LibExiv2_VERSION}" VERSION_LESS "0.27.99")

    DISABLE_GCC_COMPILER_WARNINGS("5.99.99" "-Wno-deprecated")

    # NOTE: under MacOS, this line has no effect... Why ?
    DISABLE_CLANG_COMPILER_WARNINGS("3.99.99" "-Wdeprecated-declarations")

endif()

set(libdmetadata_SRCS

    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_data_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_exif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_fileio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_comments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_iptc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_gps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_xmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_previews.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaengine_rotation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaenginesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/metaenginesettingscontainer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_video.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_motionphoto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_exiftool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_libraw.cpp

# Not used anymore as IM is very unstable with corrupted files. ExifTool can be used instead.
#    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_imagemagick.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_fileio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_comments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_template.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_exif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_iptc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_labels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_faces.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_history.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_photo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_tags.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_generic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_xmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadatasettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadatasettingscontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/geodetictools.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/containers/metadatainfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/containers/photoinfocontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/containers/videoinfocontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/containers/template.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/containers/captionvalues.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolparser_command.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolparser_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolparser_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exiftool/exiftoolprocess_p.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_canon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_exif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_fujifilm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_nikon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_olympus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_panasonic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_sony.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_extractor_xmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/focuspoint/focuspoints_writer.cpp
)

if (HEIF_FOUND)

    set(libdmetadata_SRCS
        ${libdmetadata_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/dmetadata/dmetadata_libheif.cpp
    )

endif()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(LibExiv2_FOUND)

    include_directories($<TARGET_PROPERTY:LibExiv2_INCLUDE_DIRS>)

else()

    include_directories($<TARGET_PROPERTY:exiv2lib,INTERFACE_INCLUDE_DIRECTORIES>)

endif()

# Used by digikamcore
add_library(core_dmetadata_obj OBJECT ${libdmetadata_SRCS})

target_compile_definitions(core_dmetadata_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

target_compile_definitions(core_dmetadata_obj
                           PRIVATE
                           ${EXIV2_AS_CONST_DEFINITION}
)

if(NOT Qt6_FOUND)

    set_target_properties(core_dmetadata_obj
                          PROPERTIES
                          CXX_STANDARD ${EXIV2_CXX_STANDARD}
                          CXX_STANDARD_REQUIRED YES
                          CXX_EXTENSIONS NO
    )

endif()
