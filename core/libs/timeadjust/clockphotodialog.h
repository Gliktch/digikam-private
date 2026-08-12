/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : Figure out camera clock delta from a clock picture.
 *
 * SPDX-FileCopyrightText: 2009      by Pieter Edelman <p dot edelman at gmx dot net>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QDialog>

namespace Digikam
{

class DeltaTime;

class ClockPhotoDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ClockPhotoDialog(QWidget* const parent, const QUrl& defaultUrl);
    ~ClockPhotoDialog() override;

    /**
     * Try to load the photo specified by the QUrl, and set the datetime widget
     * to the photo time. Return true on success, or false if either the photo
     * cannot be read or the datetime information can't be read.
     */
    bool setImage(const QUrl&);

    DeltaTime deltaValues() const;

private Q_SLOTS:

    void slotLoadPhoto();
    void slotOk();

    /**
     * If the cancel button is clicked, reject the dialog.
     */
    void slotCancel();

private:

    void loadSettings();
    void saveSettings();

protected:

    void showEvent(QShowEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
