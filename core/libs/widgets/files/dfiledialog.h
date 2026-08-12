/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * date        : 2017-07-04
 * Description : wrapper for the QFileDialog
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2022 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFileDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFileDialog : public QFileDialog
{
    Q_OBJECT

public:

    explicit DFileDialog(QWidget* const parent,
                         Qt::WindowFlags flags);

    explicit DFileDialog(QWidget* const parent = nullptr,
                         const QString& caption = QString(),
                         const QString& directory = QString(),
                         const QString& filter = QString());
    ~DFileDialog() override = default;

    bool hasAcceptedUrls() const;

    static QString getExistingDirectory(QWidget* const parent = nullptr,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        Options options = ShowDirsOnly);

    static QUrl getExistingDirectoryUrl(QWidget* const parent = nullptr,
                                        const QString& caption = QString(),
                                        const QUrl& dir = QUrl(),
                                        Options options = ShowDirsOnly,
                                        const QStringList& supportedSchemes = QStringList());

    static QString getOpenFileName(QWidget* const parent = nullptr,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = nullptr,
                                   Options options = Options());

    static QStringList getOpenFileNames(QWidget* const parent = nullptr,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        const QString& filter = QString(),
                                        QString* selectedFilter = nullptr,
                                        Options options = Options());

    static QUrl getOpenFileUrl(QWidget* const parent = nullptr,
                               const QString& caption = QString(),
                               const QUrl& dir = QUrl(),
                               const QString& filter = QString(),
                               QString* selectedFilter = nullptr,
                               Options options = Options(),
                               const QStringList& supportedSchemes = QStringList());

    static QList<QUrl> getOpenFileUrls(QWidget* const parent = nullptr,
                                       const QString& caption = QString(),
                                       const QUrl& dir = QUrl(),
                                       const QString& filter = QString(),
                                       QString* selectedFilter = nullptr,
                                       Options options = Options(),
                                       const QStringList& supportedSchemes = QStringList());

    static QString getSaveFileName(QWidget* const parent = nullptr,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = nullptr,
                                   Options options = Options());

    static QUrl getSaveFileUrl(QWidget* const parent = nullptr,
                               const QString& caption = QString(),
                               const QUrl& dir = QUrl(),
                               const QString& filter = QString(),
                               QString* selectedFilter = nullptr,
                               Options options = Options(),
                               const QStringList& supportedSchemes = QStringList());

private:

    static QFileDialog::Option getNativeFileDialogOption();
};

} // namespace Digikam
