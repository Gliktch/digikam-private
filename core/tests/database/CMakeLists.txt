#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Pass the path to the source video file shipped with digiKam.

add_definitions(-DVIDEO_SRC_DIR="${CMAKE_SOURCE_DIR}/core/data/pics/digikam")

if(KF${QT_VERSION_MAJOR}Notifications_FOUND)
    include_directories($<TARGET_PROPERTY:KF${QT_VERSION_MAJOR}::Notifications,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databasefields_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              digikamgui

              libmodeltest

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyfoundation_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

target_compile_definitions(privacyfoundation_utest
                           PRIVATE
                           PRIVACY_DB_CONFIG_PATH="${CMAKE_SOURCE_DIR}/core/data/database/dbconfig.xml.cmake.in"
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyprofileimport_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyanalysisgate_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyanalysiswiring_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

target_compile_definitions(privacyanalysiswiring_utest
                           PRIVATE
                           PRIVACY_ANALYSIS_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyassetinventory_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyposixfilesystemadapter_utest.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/privacyassetinventorybridge_utest.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/privacymanagedrootprovisioner_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore
                  digikamdatabase

                  ${COMMON_TEST_LINK}
    )

endif()

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystorage_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacymigrationcoordinator_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              libzip::zip

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacycategorysession_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacycasualarchive_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              libzip::zip

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacycasualpasswordrewrite_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              libzip::zip

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacytransactionjournal_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyexternalcheckouttransaction_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacypublictransition_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyproxygenerator_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              Qt${QT_VERSION_MAJOR}::Gui

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyderivativestore_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacycheckoutstore_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystrongobjectbackend_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyindependentrecovery_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystrongoriginalreader_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystrongrecoverymanifest_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystrongpasswordrewrap_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyvideoproxygenerator_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacystillitemtransaction_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              libzip::zip
              Qt${QT_VERSION_MAJOR}::Gui

              ${COMMON_TEST_LINK}
)

if(TARGET digikam-private-guard)
    add_dependencies(privacystillitemtransaction_utest digikam-private-guard)
endif()

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyruntime_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyactionpolicy_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyleaseregistry_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/privacyitemaccess_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_test(${CMAKE_CURRENT_SOURCE_DIR}/privacyfilecopyaccess_utest.cpp
             ${CMAKE_SOURCE_DIR}/core/dplugins/generic/webservices/filecopy/fctask.cpp

             TEST_NAME

             digikam-privacyfilecopyaccess_utest

             LINK_LIBRARIES

             digikamcore

             Qt${QT_VERSION_MAJOR}::Core
             Qt${QT_VERSION_MAJOR}::Widgets

             KF${QT_VERSION_MAJOR}::I18n

             ${COMMON_TEST_LINK}
)

target_include_directories(digikam-privacyfilecopyaccess_utest
                           PRIVATE
                           ${CMAKE_SOURCE_DIR}/core/dplugins/generic/webservices/filecopy)

#------------------------------------------------------------------------

ecm_add_test(${CMAKE_CURRENT_SOURCE_DIR}/privacyprintaccess_utest.cpp
             ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/printcreator/manager/advprintphoto.cpp
             ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/printcreator/manager/advprintsettings.cpp
             ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/printcreator/manager/advprinttask.cpp

             TEST_NAME

             digikam-privacyprintaccess_utest

             LINK_LIBRARIES

             digikamcore

             Qt${QT_VERSION_MAJOR}::Core
             Qt${QT_VERSION_MAJOR}::Gui
             Qt${QT_VERSION_MAJOR}::Widgets
             Qt${QT_VERSION_MAJOR}::PrintSupport

             KF${QT_VERSION_MAJOR}::ConfigCore
             KF${QT_VERSION_MAJOR}::I18n

             ${COMMON_TEST_LINK}
)

target_include_directories(digikam-privacyprintaccess_utest
                           PRIVATE
                           ${CMAKE_SOURCE_DIR}/core/dplugins/generic/tools/printcreator/manager)

set_tests_properties(digikam-privacyprintaccess_utest
                     PROPERTIES
                     ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databasesqliteinit_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              digikamgui

              ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

if (ENABLE_MYSQLSUPPORT AND ENABLE_INTERNALMYSQL)

# TODO: do not work yet.
    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databaseswitch_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore
                  digikamdatabase
                  digikamgui

                  ${COMMON_TEST_LINK}
    )

    # ---

    ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databasemysqlinit_utest.cpp

                  NAME_PREFIX

                  "digikam-"

                  LINK_LIBRARIES

                  digikamcore
                  digikamdatabase
                  digikamgui

                  ${COMMON_TEST_LINK}
    )

endif()

#------------------------------------------------------------------------

add_executable(databasetags_utest ${CMAKE_CURRENT_SOURCE_DIR}/databasetags_utest.cpp)

target_link_libraries(databasetags_utest

                      digikamcore
                      digikamdatabase
                      digikamgui

                      ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

# NOTE: this unit test fail to work as the temporary collection creation cannot be done in /tmp.
#       we need to port code to use QTest directory, as it done with metadataengine unit-tests.
#
#ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databasetags_utest.cpp
#
#              NAME_PREFIX
#
#              "digikam-"
#
#              LINK_LIBRARIES
#
#              digikamcore
#              digikamdatabase
#              digikamgui
#
#              libmodeltest
#
#              ${COMMON_TEST_LINK}
#)


add_executable(tagscache_utest ${CMAKE_CURRENT_SOURCE_DIR}/tagscache_utest.cpp)

target_link_libraries(tagscache_utest
                      digikamcore
                      digikamdatabase
                      ${COMMON_TEST_LINK}
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/haariface_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase
              digikamgui

              ${COMMON_TEST_LINK}

              GUI
)

#------------------------------------------------------------------------

ecm_add_tests(${CMAKE_CURRENT_SOURCE_DIR}/databasevideogps_utest.cpp

              NAME_PREFIX

              "digikam-"

              LINK_LIBRARIES

              digikamcore
              digikamdatabase

              ${COMMON_TEST_LINK}
)
