/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags - Private header.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"
#include "digikam_globals.h"

// Qt includes

#include <QObject>
#include <QDir>
#include <QList>
#include <QStyle>
#include <QDialog>
#include <QTimeZone>
#include <QImageReader>
#include <QImageWriter>
#include <QByteArray>
#include <QShortcut>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>
#include <QTranslator>
#include <QLibrary>
#include <QLibraryInfo>
#include <QSettings>
#include <QSysInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QTime>

// KDE includes

#include <ki18n_version.h>
#include <klocalizedstring.h>

// Helper macros to migrate KF6::Ki18n deprecated.
// To use only in implementations, not headers to not export KDE dependencies.

#if KI18N_VERSION >= QT_VERSION_CHECK(5, 89, 0)
#   include <klazylocalizedstring.h>
#   undef  I18N_NOOP
#   undef  I18NC_NOOP
#   define I18N_NOOP(x)    kli18n(x).untranslatedText()
#   define I18NC_NOOP(c,x) kli18nc(c,x)
#endif

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"
#include "rawcameradlg.h"
#include "webbrowserdlg.h"
