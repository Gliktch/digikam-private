/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : EXIF caption settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QLineEdit>

// Local includes

#include "dmetadata.h"
#include "metadataeditpage.h"

namespace Digikam
{
    class DTextEdit;
}

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class EXIFCaption : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit EXIFCaption(QWidget* const parent);
    ~EXIFCaption()                        override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncJFIFCommentIsChecked() const;
    bool syncXMPCaptionIsChecked()  const;
    bool syncIPTCCaptionIsChecked() const;

    void setCheckedSyncJFIFComment(bool c);
    void setCheckedSyncXMPCaption(bool c);
    void setCheckedSyncIPTCCaption(bool c);

    QString getEXIFUserComments()   const;

Q_SIGNALS:

    void signalModified();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
