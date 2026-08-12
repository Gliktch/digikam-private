#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/dialog
                    ${CMAKE_CURRENT_SOURCE_DIR}/exif
                    ${CMAKE_CURRENT_SOURCE_DIR}/iptc
                    ${CMAKE_CURRENT_SOURCE_DIR}/xmp
)

set(metadataeditplugin_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/exif/exifeditwidget.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exifcaption.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exifdatetime.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exifadjust.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exiflens.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exifdevice.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/exif/exiflight.cpp

                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptceditwidget.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptccontent.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptcsubjects.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptckeywords.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptccategories.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptccredits.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptcproperties.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptcstatus.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptcorigin.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/iptc/iptcenvelope.cpp

                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpeditwidget.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpcontent.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpkeywords.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpcategories.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpsubjects.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmporigin.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpcredits.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpstatus.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/xmp/xmpproperties.cpp

                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/metadatacheckbox.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/altlangstringedit.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/multistringsedit.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/multivaluesedit.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/objectattributesedit.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/metadataeditdialog.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/dialog/metadataeditpage.cpp

                            ${CMAKE_CURRENT_SOURCE_DIR}/metadataeditplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MetadataEdit
                           SOURCES ${metadataeditplugin_SRCS}
)
