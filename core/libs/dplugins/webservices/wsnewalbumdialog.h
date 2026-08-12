/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-01
 * Description : new album creation dialog for remote web service.
 *
 * SPDX-FileCopyrightText: 2010 by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRadioButton>
#include <QDateTimeEdit>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCloseEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"
#include "wsitem.h"
#include "dtextedit.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSNewAlbumDialog : public QDialog
{
    Q_OBJECT

public:

    explicit WSNewAlbumDialog(QWidget* const parent, const QString& toolName);
    ~WSNewAlbumDialog() override;

    void hideDateTime();
    void hideDesc();
    void hideLocation();

    QWidget*          getMainWidget()   const;
    QGroupBox*        getAlbumBox()     const;

    DTextEdit*        getTitleEdit()    const;
    DTextEdit*        getDescEdit()     const;
    DTextEdit*        getLocEdit()      const;
    QDateTimeEdit*    getDateTimeEdit() const;
    QDialogButtonBox* getButtonBox()    const;

    void addToMainLayout(QWidget* const widget);
    void setAlbumSuggestion(const QString& title);

private Q_SLOTS:

    void slotTextChanged();

protected:

    void getBaseAlbumProperties(WSAlbum& baseAlbum);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
