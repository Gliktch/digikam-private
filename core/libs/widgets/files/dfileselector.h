/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * date        : 2014-09-12
 * Description : a file or folder selector widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QString>
#include <QLineEdit>
#include <QPushButton>

// Local includes

#include "dlayoutbox.h"
#include "dfiledialog.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * A widget to choose a single local file or path.
 * Use line edit and file dialog properties to customize operation modes.
 */
class DIGIKAM_EXPORT DFileSelector : public DHBox
{
    Q_OBJECT

public:

    explicit DFileSelector(QWidget* const parent = nullptr);
    ~DFileSelector() override;

    QLineEdit* lineEdit() const;

    void setFileDlgPath(const QString& path);
    QString fileDlgPath() const;

    void setFileDlgMode(QFileDialog::FileMode mode);
    void setFileDlgFilter(const QString& filter);
    void setFileDlgTitle(const QString& title);
    void setFileDlgOptions(QFileDialog::Options opts);

Q_SIGNALS:

    void signalOpenFileDialog();
    void signalUrlSelected(const QUrl&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
