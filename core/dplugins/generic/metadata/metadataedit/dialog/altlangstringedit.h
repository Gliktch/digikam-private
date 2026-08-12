/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-18
 * Description : a widget to edit multiple alternative
 *               language string entries and an activation checkbox.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "altlangstredit.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class AltLangStringsEdit : public AltLangStrEdit
{
    Q_OBJECT

public:

    explicit AltLangStringsEdit(QWidget* const parent, const QString& title, const QString& desc);
    ~AltLangStringsEdit()                                   override;

    void setValid(bool v);
    bool isValid()                                   const;

    void setValues(const DMetadata::AltLangMap& values)     override;
    bool getValues(DMetadata::AltLangMap& oldValues,
                   DMetadata::AltLangMap& newValues) const;

Q_SIGNALS:

    void signalToggled(bool);
    void signalModified();
    void signalDefaultLanguageEnabled(bool);

private Q_SLOTS:

    void slotSelectionChanged(const QString& lang);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
