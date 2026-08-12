/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>
#include <QDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DProgressDlg : public QDialog
{
    Q_OBJECT

public:

    explicit DProgressDlg(QWidget* const parent = nullptr, const QString& caption=QString());
    ~DProgressDlg() override;

    void setLabel(const QString& text);
    void setTitle(const QString& text);

    int  value() const;

Q_SIGNALS:

    void signalCancelPressed();

public Q_SLOTS:

    void setMaximum(int max);
    void incrementMaximum(int added);
    void advance(int offset);
    void setValue(int value);
    void setButtonText(const QString& text);

    void addedAction(const QPixmap& icon, const QString& text);
    void reset();

protected Q_SLOTS:

    virtual void slotCancel();  // cppcheck-suppress virtualCallInConstructor

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
