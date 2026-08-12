/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-18
 * Description : XMP content settings page.
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
#include "metadataeditpage.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class XMPContent : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit XMPContent(QWidget* const parent);
    ~XMPContent()                                 override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncJFIFCommentIsChecked()         const;
    bool syncEXIFCommentIsChecked()         const;
    bool syncEXIFCopyrightIsChecked()       const;

    void setCheckedSyncJFIFComment(bool c);
    void setCheckedSyncEXIFComment(bool c);
    void setCheckedSyncEXIFCopyright(bool c);

    QString getXMPCaption()                 const;
    QString getXMPCopyright()               const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSyncCaptionOptionsEnabled(bool);
    void slotSyncCopyrightOptionsEnabled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
