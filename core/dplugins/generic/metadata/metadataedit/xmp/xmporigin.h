/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP origin settings page.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class XMPOrigin : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit XMPOrigin(QWidget* const parent);
    ~XMPOrigin()                                  override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncEXIFDateIsChecked()            const;

    void setCheckedSyncEXIFDate(bool c);

    QDateTime getXMPCreationDate()          const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSetTodayVideo();
    void slotSetTodayCreated();
    void slotSetTodayDigitalized();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
