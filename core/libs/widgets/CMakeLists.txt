#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libtextwidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/text/modelcompleter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/searchtextbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/dtextedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/dplaintextedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/dtextedit_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/localizesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/localizecontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/localizeconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/localizeselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/spellcheckconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_google.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_yandex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_bing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_libretr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_lingva.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text/donlinetranslator_tts.cpp
)

set(liblayouttools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/dexpanderbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/statesavingobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/dlayoutbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/thumbbardock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar_splitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar_dmultitabbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar_dmultitabbartab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar_dmultitabbarframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/sidebar_dmultitabbarbutton.cpp
)

set(librangetools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/range/dsliderspinbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/range/dnuminput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/range/drangebox.cpp
)

set(libcombotools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/combo/comboboxutilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/combo/dcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/combo/squeezedcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/combo/timezonecombobox.cpp
)

set(libcolorstools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/dcolorvalueselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/dhuesaturationselect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/dcolorchoosermode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/dcolorselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/colorgradientwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colors/dgradientslider.cpp
)

set(libfontstools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/dfontproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/dfontselect.cpp
)

set(libfilestools_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/files/filesaveoptionsbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/files/filesaveconflictbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/files/dfiledialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/files/dfileselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/files/dbinarysearch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/files/dbinaryiface.cpp
)

set(libmainview_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dxmlguiwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dxmlguiwindow_actions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dxmlguiwindow_fullscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dxmlguiwindow_toolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/daboutdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dactivelabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dzoombar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/dcursortracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/thememanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/thememanager_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainview/fullscreensettings.cpp
    ${CMAKE_SOURCE_DIR}/core/app/utils/digikam_globals.cpp
    ${CMAKE_SOURCE_DIR}/core/app/utils/digikam_globals_bundles.cpp
)

set(libitemview_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/dcategorizedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/dcategorizedview_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/dcategorizedsortfilterproxymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/ditemdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/ditemtooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/dcategorydrawer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/itemdelegateoverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/itemviewhoverbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/itemviewdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/itemviewtooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/itemviewcategorized.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemview/actioncategorizedview.cpp
)

set(libgraphicsviewwidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/clickdragreleaseitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/dimgchilditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/dimgpreviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/regionframeitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/regionframeitem_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/regionframeitem_events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/regionframeitem_hud.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/regionframeitem_properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/magnifieritem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview_pan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview_scroll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview_events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/graphicsdimgview_magnifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/imagezoomsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/singlephotopreviewlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/paniconwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview/itemvisibilitycontroller.cpp
)

set(libmetadatawidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/mdkeylistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/metadatalistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/metadatalistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/metadatawidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/iptcwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/exifwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/makernotewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiv2/xmpwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/ratingwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/ratingwidget_box.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/ratingwidget_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/picklabelwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/picklabelwidget_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/picklabelwidget_selector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/colorlabelwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/colorlabelwidget_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/labels/colorlabelwidget_selector.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/config/metadataselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/config/metadatapanel.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/utils/altlangstredit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/utils/altlangstredit_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/utils/countryselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/utils/subjectwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoollistviewgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoollistviewitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoollistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoolwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoolerrorview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoolloadingview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoolbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/exiftool/exiftoolconfpanel.cpp
)

set(libiccprofileswidgets_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/iccprofiles/iccpreviewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccprofiles/iccprofilewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccprofiles/cietonguewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccprofiles/iccprofilescombobox.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF${QT_VERSION_MAJOR}Sonnet_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::SonnetCore,INTERFACE_INCLUDE_DIRECTORIES>)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::SonnetUi,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Qt6_FOUND)
    include_directories(
        $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::StateMachine,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

if(KF${QT_VERSION_MAJOR}NotifyConfig_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::NotifyConfig,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

# Used by digikamcore
add_library(core_digikamwidgets_obj OBJECT
    ${libmainview_SRCS}
    ${libgraphicsviewwidgets_SRCS}
    ${libiccprofileswidgets_SRCS}
    ${libitemview_SRCS}
    ${libmetadatawidgets_SRCS}
    ${libcolorstools_SRCS}
    ${libfontstools_SRCS}
    ${libfilestools_SRCS}
    ${libcombotools_SRCS}
    ${librangetools_SRCS}
    ${liblayouttools_SRCS}
    ${libtextwidgets_SRCS}
)

target_compile_definitions(core_digikamwidgets_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

#############################################################

set(libimagehistory_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/history/filtershistorywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/itemfiltershistoryitemdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/itemfiltershistorymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/itemfiltershistorytreeitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/versionswidget.cpp
)

# Used by digikamgui
add_library(gui_imagehistorywidgets_obj OBJECT ${libimagehistory_SRCS})

target_compile_definitions(gui_imagehistorywidgets_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
