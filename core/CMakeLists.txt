#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Mohamed_Anwer, <m_dot_anwer at gmx dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NOTE: Look rules extension scripts located at core/cmake/rules/

APPLY_COMMON_POLICIES()

# ==============================================================================

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Check dependencies for ${PROJECT_NAME} version ${DIGIKAM_VERSION_STRING}")
message(STATUS "")

set(KF${QT_VERSION_MAJOR}_MIN_VERSION       "5.95.0")
set(EXIV2_MIN_VERSION                       "0.27.1")
set(OPENCV_MIN_VERSION                      "4.8.0")
set(OPENCV_MAX_VERSION                      "4.99.0")       # Not yet ported to 5.0 and later (See bug #522887)
set(LENSFUN_MIN_VERSION                     "0.2.8.0")      # For Lens auto-correction plugin
set(KSANE_MIN_VERSION                       "21.12.0")      # For digital scanner support.
set(CALENDAR_MIN_VERSION                    "5.95.0")       # Calendar Core dependency for plugin.
set(AKONADI_MIN_VERSION                     "5.95.0")       # Akonadi Contact dependency.

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

# ==============================================================================

include(RulesOptionDefinitions)

############## Find Packages ###################

# Now detect the whole Qt6 or Qt5 framework.

include(RulesQtFramework)

# KDE frameworks dependencies

include(RulesKDEFramework)

# Add rules to control the minimal Qt and KDE frameworks version 6 to support at compile time.

include(ECMDeprecationSettings)

ecm_set_disabled_deprecation_versions(
# FIXME: some Qt6 API will be removed in Qt7 eg. QSqlQuery copy constructor.
#    QT 6.9.1
    KF 6.17.0
)

# ==============================================================================
# Dependencies Rules

# mandatory

include(RulesLibJpeg)

find_package(TIFF)
find_package(PNG)
find_package(Boost)
find_package(LCMS2)
find_package(EXPAT)         # For DNGWriter: XMP SDK need Expat library to compile.
find_package(Threads)       # For DNGWriter and LibRaw which needs native threads support.
find_package(Libheif)       # For HEIF support.
find_package(Libjxl)        # For JPEG-XL support with DNGWriter.

if(HEIF_FOUND)

    find_package(X265)      # For HEIF encoding support.

    if(NOT X265_FOUND)

        set(X265_LIBRARIES "")

    endif()

endif()

# -- Exiv2 checks and adjustments ----------------------------------------------

include(RulesLibExiv2)

# -- OpenCV checks and adjustments ---------------------------------------------

include(RulesLibOpenCV)

# -- optionals -----------------------------------------------------------------

find_package(FLEX)                                 # For Panorama tool.
find_package(BISON)                                # For Panorama tool.

find_package(LibXslt)                              # For HTMLGallery tool.
find_package(LibXml2)                              # For HTMLGallery tool.

if(ENABLE_GEOLOCATION)

    set(MARBLE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/engine
                           ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/engine/geodata/data
    )

else()

    set(MARBLE_INCLUDE_DIR "")

endif()

find_package(PkgConfig)
find_package(Jasper)                               # For JPEG 2000 support.
find_package(Eigen3)                               # For Refocus tool.

# -- ImageMagick checks and adjustments ---------------------------------------

include(RulesLibMagick)

# -- X11 checks and adjustments -----------------------------------------------

include(RulesX11)

# -- OpenGL checks and adjustments --------------------------------------------

include(RulesLibOpenGL)

# -- Media player dependencies checks and adjustments -------------------------

include(RulesFFmpeg)
include(RulesMediaPlayer)

# -- Print Install Paths from ECM ---------------------------------------------

include(RulesInstallPaths)

# -- Windows compilation and linking adjustments ------------------------------

include(RulesWindows)

# -- MacOS compilation and linking adjustments --------------------------------

include(RulesMacOS)

# -- To link under Solaris (see bug #274484) ----------------------------------

if(NOT WIN32)

    find_library(MATH_LIBRARY m)

endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

    find_library(KVM_LIBRARY kvm)

endif()

# ==============================================================================
# More Optional Dependencies and Rules

find_package(GLIB2)

if(BUILD_WITH_CCACHE)

    MACRO_COMPILER_CCACHE()

endif()

# -- Check libgphoto2 library for camera devices support -----------------------

include(RulesLibgphoto2)

# -- Check LensFun library for Lens auto-correction tool -----------------------

include(RulesLiblensfun)

# -- Check dependencies for Panorama tool --------------------------------------

if(FLEX_FOUND AND BISON_FOUND AND KF${QT_VERSION_MAJOR}ThreadWeaver_FOUND)

    set(HAVE_PANORAMA 1)

else()

    set(HAVE_PANORAMA 0)

endif()

# -- Check dependencies for HTMLGallery tool -----------------------------------

if(LibXml2_FOUND AND LibXslt_FOUND)

    set(HAVE_HTMLGALLERY 1)

else()

    set(HAVE_HTMLGALLERY 0)

endif()

# -- Check dependencies for libraw ---------------------------------------------

include(RulesLibraw)

# -- Debug Symbols rules under MacOS -------------------------------------------

MACOS_DEBUG_POLICIES()

# -- Compilation options definitions -------------------------------------------

message(STATUS "--------------------------------------------------")
message(STATUS "")

if(Qt6_FOUND)

    MACRO_BOOL_TO_01(KSaneWidgets6_FOUND                        HAVE_KSANE)

else()

    MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}Sane_FOUND            HAVE_KSANE)

endif()

MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}FileMetaData_FOUND        HAVE_KFILEMETADATA)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}CalendarCore_FOUND        HAVE_KCALENDAR)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}Notifications_FOUND       HAVE_KNOTIFICATIONS)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}NotifyConfig_FOUND        HAVE_KNOTIFYCONFIG)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}KIO_FOUND                 HAVE_KIO)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}IconThemes_FOUND          HAVE_KICONTHEMES)
MACRO_BOOL_TO_01(KF${QT_VERSION_MAJOR}Sonnet_FOUND              HAVE_SONNET)
MACRO_BOOL_TO_01(LensFun_FOUND                                  HAVE_LENSFUN)
MACRO_BOOL_TO_01(Gphoto2_FOUND                                  HAVE_GPHOTO2)
MACRO_BOOL_TO_01(Jasper_FOUND                                   HAVE_JASPER)
MACRO_BOOL_TO_01(Eigen3_FOUND                                   HAVE_EIGEN3)
MACRO_BOOL_TO_01(ENABLE_GEOLOCATION                             HAVE_GEOLOCATION)
MACRO_BOOL_TO_01(ENABLE_AKONADICONTACTSUPPORT                   HAVE_AKONADICONTACT)
MACRO_BOOL_TO_01(ENABLE_MYSQLSUPPORT                            HAVE_MYSQLSUPPORT)
MACRO_BOOL_TO_01(ENABLE_INTERNALMYSQL                           HAVE_INTERNALMYSQL)
MACRO_BOOL_TO_01(ENABLE_MEDIAPLAYER                             HAVE_MEDIAPLAYER)
MACRO_BOOL_TO_01(ENABLE_QTMULTIMEDIA                            HAVE_QTMULTIMEDIA)
MACRO_BOOL_TO_01(ENABLE_DBUS                                    HAVE_DBUS)
MACRO_BOOL_TO_01(ENABLE_APPSTYLES                               HAVE_APPSTYLE_SUPPORT)
MACRO_BOOL_TO_01(ImageMagick_FOUND                              HAVE_IMAGE_MAGICK)
MACRO_BOOL_TO_01(HEIF_FOUND                                     HAVE_HEIF)
MACRO_BOOL_TO_01(X265_FOUND                                     HAVE_X265)
MACRO_BOOL_TO_01(JXL_FOUND                                      HAVE_JXL)
MACRO_BOOL_TO_01(Qt5XmlPatterns_FOUND                           HAVE_QTXMLPATTERNS)
MACRO_BOOL_TO_01(CCACHE_FOUND                                   HAVE_CCACHE)

# Set config files accordingly with optional dependencies

configure_file(app/utils/digikam_config.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_config.h)

# ==============================================================================
# Log messages

message(STATUS "")
message(STATUS "----------------------------------------------------------------------------------")
message(STATUS " digiKam ${DIGIKAM_VERSION_STRING} dependencies results   <https://www.digikam.org>")
message(STATUS "")

PRINT_COMPONENT_COMPILE_STATUS("MySQL Database Support"                     ENABLE_MYSQLSUPPORT)
PRINT_COMPONENT_COMPILE_STATUS("MySQL Internal Support"                     ENABLE_INTERNALMYSQL)
PRINT_COMPONENT_COMPILE_STATUS("Showfoto Support"                           ENABLE_SHOWFOTO)
PRINT_COMPONENT_COMPILE_STATUS("DBUS Support"                               ENABLE_DBUS)
PRINT_COMPONENT_COMPILE_STATUS("App. Style Support"                         ENABLE_APPSTYLES)
PRINT_COMPONENT_COMPILE_STATUS("Geolocation Support"                        ENABLE_GEOLOCATION)
PRINT_COMPONENT_COMPILE_STATUS("Media Player Support"                       ENABLE_MEDIAPLAYER)
PRINT_COMPONENT_COMPILE_STATUS("QtMultimedia Support"                       ENABLE_QTMULTIMEDIA)

# ==============================================================================

PRINT_LIBRARY_STATUS("libboostgraph" "https://github.com/boostorg/boost"                 "(version >= 1.43.0)"                Boost_FOUND)

if(LibExiv2_FOUND)

    PRINT_LIBRARY_STATUS("LibExiv2"  "https://github.com/Exiv2/exiv2"                    "(version >= ${EXIV2_MIN_VERSION}"   LibExiv2_FOUND)

else()

    PRINT_LIBRARY_STATUS("libexiv2"  "https://github.com/Exiv2/exiv2"                    "(version >= ${EXIV2_MIN_VERSION}"   exiv2_FOUND)

endif()

PRINT_LIBRARY_STATUS("libexpat"      "https://libexpat.github.io"                        "(version >= 2.0.0)"                 EXPAT_FOUND)
PRINT_LIBRARY_STATUS("libjpeg"       "https://github.com/libjpeg-turbo/libjpeg-turbo"    "(version >= 6b)"                    JPEG_FOUND)
PRINT_LIBRARY_STATUS("libkde"        "https://invent.kde.org/frameworks/"                "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"    KF${QT_VERSION_MAJOR}_FOUND)
PRINT_LIBRARY_STATUS("liblcms"       "https://github.com/mm2/Little-CMS"                 "(version >= 2.0.0)"                 LCMS2_FOUND)
PRINT_LIBRARY_STATUS("libopencv"     "https://github.com/opencv/opencv"                  "(version >= ${OPENCV_MIN_VERSION})" OpenCV_FOUND)
PRINT_LIBRARY_STATUS("libpng"        "https://libpng.sourceforge.io/"                    "(version >= 1.2.7)"                 PNG_FOUND)
PRINT_LIBRARY_STATUS("libpthread"    "https://www.gnu.org/software/hurd/libpthread.html" "(version >= 2.0.0)"                 CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)
PRINT_LIBRARY_STATUS("libqt"         "https://code.qt.io/cgit/qt/qtbase.git/"            "(version >= ${QT_MIN_VERSION})"     Qt${QT_VERSION_MAJOR}_FOUND)
PRINT_LIBRARY_STATUS("libtiff"       "https://gitlab.com/libtiff/libtiff/"               "(version >= 3.8.2)"                 TIFF_FOUND)

# ==============================================================================

PRINT_OPTIONAL_LIBRARY_STATUS("bison"             "https://www.gnu.org/software/bison/bison.html"                   "(version >= 2.5.0)"                    "digiKam will be compiled without Panorama support."                          BISON_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("ccache"            "https://ccache.dev"                                              "(version >= 3.0.0)"                    "digiKam will be compiled without CCACHE build support."                      HAVE_CCACHE)
PRINT_OPTIONAL_LIBRARY_STATUS("flex"              "https://github.com/westes/flex"                                  "(version >= 2.5.0)"                    "digiKam will be compiled without Panorama support."                          FLEX_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libakonadicontact" "https://invent.kde.org/pim/akonadi-contacts"                     "(version >= ${AKONADI_MIN_VERSION})"   "digiKam will be compiled without KDE desktop address book support."          KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND OR KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libimagemagick"    "https://github.com/ImageMagick/ImageMagick.git"                  "(version >= 6.7.0)"                    "digiKam will be compiled without ImageMagick codecs."                        HAVE_IMAGE_MAGICK)
PRINT_OPTIONAL_LIBRARY_STATUS("libeigen3"         "https://github.com/eigenteam/eigen-git-mirror"                   "(version >= 3.0.0)"                    "digiKam will be compiled without Refocus tool support."                      Eigen3_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libgphoto2"        "https://github.com/gphoto/libgphoto2"                            "(version >= 2.4.0)"                    "digiKam will be compiled without GPhoto2 camera drivers support."            Gphoto2_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libjasper"         "https://github.com/mdadams/jasper"                               "(version >= 1.7.0)"                    "digiKam will be compiled without JPEG2000 support."                          Jasper_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libjpegxl"         "https://github.com/libjxl/libjxl"                                "(version >= 0.7)"                      "digiKam will be compiled without DNG support."                               JXL_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkcalendarcore"  "https://invent.kde.org/frameworks/kcalendarcore"                 "(version >= ${CALENDAR_MIN_VERSION})"  "digiKam will be compiled without advanced calendar support."                 KF${QT_VERSION_MAJOR}CalendarCore_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkfilemetadata"  "https://invent.kde.org/frameworks/kfilemetadata"                 "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without KDE desktop file metadata support."         KF${QT_VERSION_MAJOR}FileMetaData_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkiconthemes"    "https://invent.kde.org/frameworks/kiconthemes"                   "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without KDE desktop icon themes support."           KF${QT_VERSION_MAJOR}IconThemes_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libkio"            "https://invent.kde.org/frameworks/kio"                           "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without KDE desktop integration support."           KF${QT_VERSION_MAJOR}KIO_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libknotifications" "https://invent.kde.org/frameworks/knotifyconfig"                 "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without KDE desktop notifications support."         KF${QT_VERSION_MAJOR}Notifications_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libknotifyconfig"  "https://invent.kde.org/frameworks/knotifications"                "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without KDE desktop notify configuration support."  KF${QT_VERSION_MAJOR}NotifyConfig_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libsonnet"         "https://invent.kde.org/frameworks/sonnet"                        "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without spell-checking support."                    KF${QT_VERSION_MAJOR}Sonnet_FOUND)

if(Qt6_FOUND)

    PRINT_OPTIONAL_LIBRARY_STATUS("libksane"      "https://invent.kde.org/graphics/libksane"                        "(version >= ${KSANE_MIN_VERSION})"     "digiKam will be compiled without flat scanners support."                     KSaneWidgets6_FOUND)

else()

    PRINT_OPTIONAL_LIBRARY_STATUS("libksane"      "https://invent.kde.org/graphics/libksane"                        "(version >= ${KSANE_MIN_VERSION})"     "digiKam will be compiled without flat scanners support."                     KF${QT_VERSION_MAJOR}Sane_FOUND)

endif()

PRINT_OPTIONAL_LIBRARY_STATUS("liblensfun"        "https://github.com/lensfun/lensfun"                              "(version >= 0.2.6)"                    "digiKam will be compiled without Lens Auto Correction tool support."         LensFun_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libglib2"          "https://developer.gnome.org/glib/"                               "(version >= 2.0.0)"                    "digiKam will be compiled without Contents Aware Resizer tool support."       GLIB2_FOUND)

PRINT_OPTIONAL_LIBRARY_STATUS("libthreadweaver"   "https://invent.kde.org/frameworks/threadweaver"                  "(version >= ${KF${QT_VERSION_MAJOR}_MIN_VERSION})"       "digiKam will be compiled without Panorama support."                          KF${QT_VERSION_MAJOR}ThreadWeaver_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libxml2"           "https://gitlab.gnome.org/GNOME/libxml2/"                         "(version >= 2.7.0)"                    "digiKam will be compiled without HTMLGallery support."                       LibXml2_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libxslt"           "https://gitlab.gnome.org/GNOME/libxslt"                          "(version >= 1.1.0)"                    "digiKam will be compiled without HTMLGallery support."                       LibXslt_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libheif"           "https://github.com/strukturag/libheif"                           "(version >= 1.6.0)"                    "digiKam will be compiled without HEIF support."                              HEIF_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("libx265"           "https://github.com/videolan/x265.git"                            "(version >= 2.2)"                      "digiKam will be compiled without HEIF encoding support."                     X265_FOUND)
PRINT_OPTIONAL_LIBRARY_STATUS("OpenGL"            "https://gitlab.freedesktop.org/mesa/mesa/"                       "(version >= 11.0.0)"                   "digiKam will be compiled without OpenGL support."                            HAVE_OPENGL)

if(NOT Qt6_FOUND)

    # NOTE: This module have been removed with Qt6.

    PRINT_OPTIONAL_LIBRARY_STATUS("libqtxmlpatterns"  "https://code.qt.io/cgit/qt/qtxmlpatterns.git/"                   "(version >= ${QT_MIN_VERSION})"        "digiKam will be compiled without Rajce tool support."                        HAVE_QTXMLPATTERNS)

endif()

# ==============================================================================

if(Boost_FOUND                          AND
   (LibExiv2_FOUND OR exiv2_FOUND)      AND
   EXPAT_FOUND                          AND
   JPEG_FOUND                           AND
   KF${QT_VERSION_MAJOR}_FOUND          AND
   LCMS2_FOUND                          AND
   OpenCV_FOUND                         AND
   PNG_FOUND                            AND
   Qt${QT_VERSION_MAJOR}_FOUND          AND
   TIFF_FOUND                           AND
   Threads_FOUND                        AND
   (Qt${QT_VERSION_MAJOR}Test_FOUND OR NOT BUILD_TESTING) AND
   (CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)
  )

    message(STATUS " digiKam can be compiled.................. YES")
    set(DIGIKAM_CAN_BE_COMPILED true)

else()

    message(FATAL_ERROR " digiKam will be compiled.................. NO  (Check dependencies from the developer documentation https://files.kde.org/digikam/api/)")
    set(DIGIKAM_CAN_BE_COMPILED false)

endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "")

if(DIGIKAM_CAN_BE_COMPILED)

    include(MacroGitHeader)
    GIT_HEADER()

    include(MacroBuildDateHeader)
    BUILD_DATE_HEADER()

    # ==========================================================================
    # To prevent warnings from M$ compiler

    if(WIN32 AND MSVC)

        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_ATL_SECURE_NO_WARNINGS)
        add_definitions(-D_AFX_SECURE_NO_WARNINGS)

    endif()

    # ==========================================================================
    # To use ccache with compiler

    if(HAVE_CCACHE)

        message(STATUS "Using ccache to speed-up compilations..... YES")
        add_definitions(-DCCACHE_VERSION="${CCACHE_VERSION}")

    else()

        message(STATUS "Using ccache to speed-up compilations..... NO")

    endif()

    # ==========================================================================
    # Definitions rules

    # TODO: revise dropped compiler flags accordingly with patched source code which fix compilation

    # Remove GCC flags which generate a lots of compilation warnings
    REMOVE_GCC_COMPILER_WARNINGS("-Wsuggest-override")
    REMOVE_GCC_COMPILER_WARNINGS("-Wcast-align")
    REMOVE_GCC_COMPILER_WARNINGS("-Wzero-as-null-pointer-constant")
    REMOVE_GCC_COMPILER_WARNINGS("-pedantic")

    # See bug for NETBSD #501769
    REMOVE_GCC_COMPILER_WARNINGS("-Werror=undef")

    # Remove deprecated-copy GCC warnings which generate a lots of compilation warnings
    DISABLE_GCC_COMPILER_WARNINGS("8.99.99" "-Wno-deprecated-copy")

    # Remove inconsistent-missing-override Clang warnings which generate a lots of compilation warnings
    DISABLE_CLANG_COMPILER_WARNINGS("7.99.99" "-Wno-inconsistent-missing-override")

    # Remove Clang flags which generate a lots of compilation warnings
    REMOVE_CLANG_COMPILER_WARNINGS("-Wzero-as-null-pointer-constant")
    REMOVE_CLANG_COMPILER_WARNINGS("-Wc11-extensions")
    REMOVE_CLANG_COMPILER_WARNINGS("-Wc++23-attribute-extensions")

    include(MacroSanitizers)
    ENABLE_COMPILER_SANITIZERS()

    # translations catalog
    add_definitions(-DTRANSLATION_DOMAIN=\"digikam\")

    # NOTE: with libpgf 6.11.24 OpenMP is not very well supported. We disable
    # it to be safe. See B.K.O #273765 for details.
    add_definitions(-DLIBPGF_DISABLE_OPENMP)

    # For C++17 and later compatibility LCMS2 must not use register keyword.
    add_definitions(-DCMS_NO_REGISTER_KEYWORD=1)

    # Enable C++ Exceptions support, require by Greycstoration algorithm
    # (CImg.h) and PGF codec
    kde_enable_exceptions()

    # Enforce modern Qt code
    add_definitions(-DQT_DEPRECATED_WARNINGS
                    -DQT_USE_QSTRINGBUILDER
                    -DQT_NO_CAST_TO_ASCII
                    -DQT_NO_CAST_FROM_ASCII
                    -DQT_NO_CAST_FROM_BYTEARRAY
                    -DQT_NO_URL_CAST_FROM_STRING
                    -DQT_STRICT_ITERATORS
                    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
                    -DQT_NO_KEYWORDS
                    -DQT_NO_FOREACH
    )

    # -----------------------------------------------------------------------------------
    # Add defines for Apple package manager

    if(APPLE)

        if("${DK_APPLE_PACKAGE_MANAGER}" STREQUAL "homebrew")

            add_compile_definitions(DK_APPLE_PACKAGE_HOMEBREW)

        else()

            add_compile_definitions(DK_APPLE_PACKAGE_MACPORTS)

        endif()

    endif()

    # ==========================================================================
    # Includes rules

    # Recursively get all directories which contain header files
    # NOTE: Do not parse more than expected root directories to prevent broken
    # compilation under Windows as include path length is limited in opposite of POSIX systems.

    set(DK_INCLUDES_ALL "")

    HEADER_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs DK_LOCAL_INCLUDES_RAW)

    # This macro will set all paths which do not contain libjpeg-, QtAVPlayer, and qtmm.
    # We will add later the directory we need

    foreach(var ${DK_LOCAL_INCLUDES_RAW})

        # Filter internal 3rd party library paths to not share with the rest of codes

        string(REGEX MATCH "libjpeg/"          _item_libjpeg    ${var})
        string(REGEX MATCH "webservices/o2/"   _item_libo2      ${var})
        string(REGEX MATCH "extra/dng_sdk"     _item_libdng     ${var})
        string(REGEX MATCH "extra/md5"         _item_libmd5     ${var})
        string(REGEX MATCH "extra/xmp_sdk"     _item_libxmp     ${var})
        string(REGEX MATCH "pgfutils/libpgf"   _item_libpgf     ${var})
        string(REGEX MATCH "rawengine/libraw/" _item_libraw     ${var})
        string(REGEX MATCH "/QtAVPlayer"       _item_qtavplayer ${var})
        string(REGEX MATCH "/qtmm"             _item_qtmm       ${var})
        string(REGEX MATCH "/lqr"              _item_lqr        ${var})

        if(
           _item_libjpeg    STREQUAL "" AND
           _item_libo2      STREQUAL "" AND
           _item_libdng     STREQUAL "" AND
           _item_libmd5     STREQUAL "" AND
           _item_libxmp     STREQUAL "" AND
           _item_libpgf     STREQUAL "" AND
           _item_libraw     STREQUAL "" AND
           _item_qtavplayer STREQUAL "" AND
           _item_qtmm       STREQUAL "" AND
           _item_lqr        STREQUAL ""
          )

            list(APPEND DK_LOCAL_INCLUDES ${var})

        endif()

    endforeach()

    set(DK_LOCAL_INCLUDES ${DK_LOCAL_INCLUDES}
                          ${CMAKE_CURRENT_SOURCE_DIR}/libs/jpegutils/${DIGIKAM_LIBJPEG_DIR}
    )

    # for config headers digikam_version.h digikam_gitversion.h digikam_config.h
    # digikam_dbconfig.h digikam_opencv.h digikam_export.h

    set(DK_LOCAL_INCLUDES ${DK_LOCAL_INCLUDES}
                          ${CMAKE_CURRENT_SOURCE_DIR}/app/utils
                          ${CMAKE_CURRENT_BINARY_DIR}/app/utils
    )

    # ==========================================================================
    # Common targets parts

    add_subdirectory(data)
    add_subdirectory(libs)
    add_subdirectory(utilities)
    add_subdirectory(app)
    add_subdirectory(dplugins)

    if(ENABLE_SHOWFOTO)
        add_subdirectory(showfoto)
    endif()

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()

endif()
