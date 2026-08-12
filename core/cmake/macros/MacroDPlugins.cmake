# Some useful macros for printing status information
#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(CMakeParseArguments)

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link a Generic DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_GENERIC_PLUGIN(NAME    _plugin_name_
#                                   SOURCE  _plugin_sources_
#                                   DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a Generic plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Generic_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_GENERIC_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(NOT _parse_results_NAME)
        message(FATAL_ERROR "Generic plugin name is required.")
    endif()

    if(NOT _parse_results_SOURCES)
        message(FATAL_ERROR "Generic plugin sources is required.")
    endif()

    if(APPLE)
        set(_extra_deps /System/Library/Frameworks/AppKit.framework
                        /System/Library/Frameworks/IOKit.framework
        )
    endif()

    add_library(Generic_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Generic_${_parse_results_NAME}_Plugin

                          PRIVATE

                          digikamcore

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Xml
                          Qt${QT_VERSION_MAJOR}::Widgets
                          Qt${QT_VERSION_MAJOR}::Network
                          Qt${QT_VERSION_MAJOR}::NetworkAuth

                          KF${QT_VERSION_MAJOR}::XmlGui
                          KF${QT_VERSION_MAJOR}::I18n
                          KF${QT_VERSION_MAJOR}::ConfigCore
                          KF${QT_VERSION_MAJOR}::Service

                          ${_parse_results_DEPENDS}
                          ${_extra_deps}
    )

    install(TARGETS Generic_${_parse_results_NAME}_Plugin
            DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/generic
    )

    # Install debug symbols

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:Generic_${_parse_results_NAME}_Plugin>"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/generic
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

    if(APPLE)
        install(FILES "$<TARGET_FILE:Generic_${_parse_results_NAME}_Plugin>.dSYM"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/generic
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link an Editor DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_EDITOR_PLUGIN(NAME    _plugin_name_
#                                  SOURCE  _plugin_sources_
#                                  DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default an Editor plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Editor_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_EDITOR_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(NOT _parse_results_NAME)
        message(FATAL_ERROR "Editor plugin name is required.")
    endif()

    if(NOT _parse_results_SOURCES)
        message(FATAL_ERROR "Editor plugin sources is required.")
    endif()

    if(APPLE)
        set(_extra_deps /System/Library/Frameworks/AppKit.framework
                        /System/Library/Frameworks/IOKit.framework
        )
    endif()

    add_library(Editor_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Editor_${_parse_results_NAME}_Plugin

                          PRIVATE

                          digikamcore

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Xml
                          Qt${QT_VERSION_MAJOR}::Widgets

                          KF${QT_VERSION_MAJOR}::XmlGui
                          KF${QT_VERSION_MAJOR}::I18n
                          KF${QT_VERSION_MAJOR}::ConfigCore
                          KF${QT_VERSION_MAJOR}::Service

                          ${_parse_results_DEPENDS}
                          ${_extra_deps}
    )

    install(TARGETS Editor_${_parse_results_NAME}_Plugin
            DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/editor
    )

    # Install debug symbols

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:Editor_${_parse_results_NAME}_Plugin>"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/editor
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

    if(APPLE)
        install(FILES "$<TARGET_FILE:Editor_${_parse_results_NAME}_Plugin>.dSYM"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/editor
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link an Bqm DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_BQM_PLUGIN(NAME    _plugin_name_
#                               SOURCE  _plugin_sources_
#                               DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a Bqm plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# Bqm_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_BQM_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(NOT _parse_results_NAME)
        message(FATAL_ERROR "Bqm plugin name is required.")
    endif()

    if(NOT _parse_results_SOURCES)
        message(FATAL_ERROR "Bqm plugin sources is required.")
    endif()

    if(APPLE)
        set(_extra_deps /System/Library/Frameworks/AppKit.framework
                        /System/Library/Frameworks/IOKit.framework
        )
    endif()

    add_library(Bqm_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(Bqm_${_parse_results_NAME}_Plugin

                          PRIVATE

                          digikamcore
                          digikamdatabase
                          digikamgui

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Xml
                          Qt${QT_VERSION_MAJOR}::Widgets
                          Qt${QT_VERSION_MAJOR}::Sql

                          KF${QT_VERSION_MAJOR}::XmlGui
                          KF${QT_VERSION_MAJOR}::I18n
                          KF${QT_VERSION_MAJOR}::ConfigCore
                          KF${QT_VERSION_MAJOR}::Service

                          ${_parse_results_DEPENDS}
                          ${_extra_deps}
    )

    install(TARGETS Bqm_${_parse_results_NAME}_Plugin
            DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/bqm
    )

    # Install debug symbols

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:Bqm_${_parse_results_NAME}_Plugin>"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/bqm
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

    if(APPLE)
        install(FILES "$<TARGET_FILE:Bqm_${_parse_results_NAME}_Plugin>.dSYM"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/bqm
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link an RawImport DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_RAWIMPORT_PLUGIN(NAME    _plugin_name_
#                                     SOURCE  _plugin_sources_
#                                     DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a RawImport plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# RawImport_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_RAWIMPORT_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(NOT _parse_results_NAME)
        message(FATAL_ERROR "RawImport plugin name is required.")
    endif()

    if(NOT _parse_results_SOURCES)
        message(FATAL_ERROR "RawImport plugin sources is required.")
    endif()

    if(APPLE)
        set(_extra_deps /System/Library/Frameworks/AppKit.framework
                        /System/Library/Frameworks/IOKit.framework
        )
    endif()

    add_library(RawImport_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(RawImport_${_parse_results_NAME}_Plugin

                          PRIVATE

                          digikamcore

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Xml
                          Qt${QT_VERSION_MAJOR}::Widgets

                          KF${QT_VERSION_MAJOR}::XmlGui
                          KF${QT_VERSION_MAJOR}::I18n
                          KF${QT_VERSION_MAJOR}::ConfigCore
                          KF${QT_VERSION_MAJOR}::Service

                          ${_parse_results_DEPENDS}
                          ${_extra_deps}
    )

    install(TARGETS RawImport_${_parse_results_NAME}_Plugin
            DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/rawimport
    )

    # Install debug symbols

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:RawImport_${_parse_results_NAME}_Plugin>"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/rawimport
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

    if(APPLE)
        install(FILES "$<TARGET_FILE:RawImport_${_parse_results_NAME}_Plugin>.dSYM"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/rawimport
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endmacro()

# -------------------------------------------------------------------------

# This macro implement the rules to compile and link a DImg DPlugin with extra arguments.
#
# Usage: DIGIKAM_ADD_DIMG_PLUGIN(NAME    _plugin_name_
#                                SOURCE  _plugin_sources_
#                                DEPENDS _plugin_dependencies_)
#
# With: _plugin_name_ the literal name of the plugin (mandatory).
#       _plugin_sources_ the list of source codes to compile (mandatory).
#       _plugin_dependencies_ the list of dependencies to link (optional).
#
# Note: by default a DImg plugin is linked with digikamcore shared library.
#
# This macro will generate a plugin library with this pattern as file name:
# DImg_${_pluginname_}_Plugin
#
macro(DIGIKAM_ADD_DIMG_PLUGIN)

    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS)
    set(_MULTI_VALUE_ARGS NAME SOURCES DEPENDS)

    cmake_parse_arguments(_parse_results "${_OPTIONS_ARGS}"
                                         "${_ONE_VALUE_ARGS}"
                                         "${_MULTI_VALUE_ARGS}"
                                         ${ARGN}
    )

    # Mandatory
    if(NOT _parse_results_NAME)
        message(FATAL_ERROR "DImg plugin name is required.")
    endif()

    if(NOT _parse_results_SOURCES)
        message(FATAL_ERROR "DImg plugin sources is required.")
    endif()

    if(APPLE)
        set(_extra_deps /System/Library/Frameworks/AppKit.framework
                        /System/Library/Frameworks/IOKit.framework
        )
    endif()

    add_library(DImg_${_parse_results_NAME}_Plugin
                MODULE ${_parse_results_SOURCES})

    target_link_libraries(DImg_${_parse_results_NAME}_Plugin

                          PRIVATE

                          digikamcore

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Xml
                          Qt${QT_VERSION_MAJOR}::Widgets

                          KF${QT_VERSION_MAJOR}::I18n
                          KF${QT_VERSION_MAJOR}::ConfigCore

                          ${_parse_results_DEPENDS}
                          ${_extra_deps}
    )

    install(TARGETS DImg_${_parse_results_NAME}_Plugin
            DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/dimg
    )

    # Install debug symbols

    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:DImg_${_parse_results_NAME}_Plugin>"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/dimg
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

    if(APPLE)
        install(FILES "$<TARGET_FILE:DImg_${_parse_results_NAME}_Plugin>.dSYM"
                DESTINATION ${KDE_INSTALL_FULL_PLUGINDIR}/digikam/dimg
                CONFIGURATIONS Debug RelWithDebInfo
        )
    endif()

endmacro()
