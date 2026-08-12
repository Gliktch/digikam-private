#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(KF${QT_VERSION_MAJOR} ${KF${QT_VERSION_MAJOR}_MIN_VERSION} REQUIRED
                                    COMPONENTS
                                    XmlGui
                                    CoreAddons
                                    Config
                                    Service
                                    WindowSystem
                                    Solid
                                    I18n
)

find_package(KF${QT_VERSION_MAJOR} ${KF${QT_VERSION_MAJOR}_MIN_VERSION} QUIET
                                    OPTIONAL_COMPONENTS
                                    KIO                         # For Desktop integration (Widgets only).
                                    IconThemes                  # For Desktop integration.
                                    ThreadWeaver                # For Panorama tool.
                                    NotifyConfig                # Plasma desktop application notify configuration.
                                    Notifications               # Plasma desktop notifications integration.
                                    Sonnet                      # For spell-checking.
)

if(ENABLE_KFILEMETADATASUPPORT)

    find_package(KF${QT_VERSION_MAJOR} ${KF${QT_VERSION_MAJOR}_MIN_VERSION} QUIET
                                        OPTIONAL_COMPONENTS
                                        FileMetaData            # For Plasma desktop file indexer support.
    )

endif()

if(ENABLE_AKONADICONTACTSUPPORT)

    if(Qt6_FOUND)

        find_package(KPim${QT_VERSION_MAJOR} ${AKONADI_MIN_VERSION} QUIET
                                                OPTIONAL_COMPONENTS
                                                Akonadi
                                                AkonadiContactCore  # For KDE Mail Contacts support.
        )
        find_package(KF${QT_VERSION_MAJOR} ${AKONADI_MIN_VERSION} QUIET
                                                OPTIONAL_COMPONENTS
                                                Contacts            # API for contacts/address book data.
        )
        find_package(KPim${QT_VERSION_MAJOR}Akonadi ${AKONADI_MIN_VERSION} QUIET)
        find_package(KPim${QT_VERSION_MAJOR}AkonadiContactCore ${AKONADI_MIN_VERSION} QUIET)

    else()

        find_package(KF${QT_VERSION_MAJOR} ${AKONADI_MIN_VERSION} QUIET
                                                OPTIONAL_COMPONENTS
                                                Akonadi
                                                AkonadiContact      # For KDE Mail Contacts support.
                                                Contacts            # API for contacts/address book data.
        )
        find_package(KPim${QT_VERSION_MAJOR}Akonadi ${AKONADI_MIN_VERSION} QUIET)
        find_package(KPim${QT_VERSION_MAJOR}AkonadiContact ${AKONADI_MIN_VERSION} QUIET)

    endif()

endif()

if(Qt6_FOUND)

    find_package(KSaneWidgets6)

else()

    find_package(KF${QT_VERSION_MAJOR} ${KSANE_MIN_VERSION} QUIET
                                       OPTIONAL_COMPONENTS
                                       Sane                      # For digital scanner support.
    )

endif()

find_package(KF${QT_VERSION_MAJOR} ${CALENDAR_MIN_VERSION} QUIET
                                         OPTIONAL_COMPONENTS
                                         CalendarCore           # For Calendar tool.
)

if ("${KF${QT_VERSION_MAJOR}CalendarCore_VERSION}" VERSION_GREATER 5.6.40)

    set(HAVE_KCALENDAR_QDATETIME TRUE)

endif()

if(ENABLE_AKONADICONTACTSUPPORT AND
   (NOT (KPim${QT_VERSION_MAJOR}AkonadiContact_FOUND OR KPim${QT_VERSION_MAJOR}AkonadiContactCore_FOUND) OR NOT KF${QT_VERSION_MAJOR}Contacts_FOUND))

    set(ENABLE_AKONADICONTACTSUPPORT OFF)

endif()

if(ENABLE_KFILEMETADATASUPPORT AND NOT KF${QT_VERSION_MAJOR}FileMetaData_FOUND)

    set(ENABLE_KFILEMETADATASUPPORT OFF)

endif()

# Check if KIO have been compiled with KIOWidgets. digiKam only needs this one.

if(ENABLE_KIO)

    if(KF${QT_VERSION_MAJOR}KIO_FOUND)

        get_target_property(KIOWidgets_INCLUDE_DIRS KF${QT_VERSION_MAJOR}::KIOWidgets
                            INTERFACE_INCLUDE_DIRECTORIES)
        message(STATUS "KF${QT_VERSION_MAJOR}::KIOWidgets include dirs: ${KIOWidgets_INCLUDE_DIRS}")

        if(NOT KIOWidgets_INCLUDE_DIRS)

            message(STATUS "KF${QT_VERSION_MAJOR}::KIOWidgets not available in shared KIO library. KIO support disabled.")
            set(KF${QT_VERSION_MAJOR}KIO_FOUND FALSE)

        endif()

    endif()

else()

    message(STATUS "KF${QT_VERSION_MAJOR}::KIO support is explicitly disabled.")
    set(KF${QT_VERSION_MAJOR}KIO_FOUND FALSE)

endif()
