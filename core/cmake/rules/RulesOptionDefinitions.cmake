#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

option(ENABLE_KFILEMETADATASUPPORT       "Build digiKam with Plasma desktop files indexer support (default=OFF)"              OFF)
option(ENABLE_AKONADICONTACTSUPPORT      "Build digiKam with Plasma desktop Mail Contacts support (default=OFF)"              OFF)
option(ENABLE_GEOLOCATION                "Build digiKam with Geolocation support (default=ON)"                                ON)
option(ENABLE_MEDIAPLAYER                "Build digiKam with Media Player support (default=ON)"                               ON)
option(ENABLE_DBUS                       "Build digiKam with DBUS support (default=ON)"                                       ON)
option(ENABLE_APPSTYLES                  "Build digiKam with support for changing the widget application style (default=OFF)" OFF)
option(ENABLE_KIO                        "Build digiKam with KIO support (default=ON)"                                        ON)

# Database options:
option(ENABLE_MYSQLSUPPORT               "Build digiKam with MySQL dabatase support (default=ON)"                             ON)
option(ENABLE_INTERNALMYSQL              "Build digiKam with internal MySQL server executable (default=ON)"                   ON)

# Showfoto application
option(ENABLE_SHOWFOTO                   "Build Showfoto stand-alone image editor (default=ON)"                               ON)

# Developer options:
option(ENABLE_DIGIKAM_MODELTEST          "Enable ModelTest on some models for debugging (default=OFF)"                        OFF)
option(ENABLE_SANITIZERS                 "Enable ASAN and UBSAN sanitizers when available (default=OFF)"                      OFF)
option(BUILD_WITH_CCACHE                 "Use ccache to speed up compilations"                                                OFF)
option(BUILD_WITH_QT6                    "Build with Qt6, else Qt5"                                                           OFF)
