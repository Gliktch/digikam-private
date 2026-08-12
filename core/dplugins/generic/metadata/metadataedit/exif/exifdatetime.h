/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF date and time settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QDateTime>

// Local includes

#include "dmetadata.h"
#include "metadataeditpage.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class EXIFDateTime : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit EXIFDateTime(QWidget* const parent);
    ~EXIFDateTime() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncXMPDateIsChecked() const;
    bool syncIPTCDateIsChecked() const;

    void setCheckedSyncXMPDate(bool c);
    void setCheckedSyncIPTCDate(bool c);

    QDateTime getEXIFCreationDate() const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSetTodayCreated();
    void slotSetTodayOriginal();
    void slotSetTodayDigitalized();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
