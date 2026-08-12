/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : caption editor
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QDateTime>
#include <QLineEdit>

// Local includes

#include "digikam_export.h"
#include "dlayoutbox.h"
#include "captionvalues.h"
#include "altlangstredit.h"

namespace Digikam
{

class DIGIKAM_EXPORT CaptionEdit : public DVBox
{
    Q_OBJECT

public:

    explicit CaptionEdit(QWidget* const parent);
    ~CaptionEdit()                          override;

    void setValues(const CaptionsMap& values);
    CaptionsMap& values()            const;

    void setPlaceholderText(const QString& msg);

    void setCurrentLanguageCode(const QString& lang);
    QString currentLanguageCode()    const;

    void reset();

    AltLangStrEdit* altLangStrEdit() const;
    QLineEdit* authorEdit()          const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSelectionChanged(const QString&);
    void slotCaptionModified(const QString&, const QString&);
    void slotAddValue(const QString&, const QString&);
    void slotDeleteValue(const QString&);
    void slotAuthorChanged(const QString&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
