/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC keywords settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class IPTCKeywords : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit IPTCKeywords(QWidget* const parent);
    ~IPTCKeywords() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotKeywordSelectionChanged();
    void slotAddKeyword();
    void slotDelKeyword();
    void slotRepKeyword();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
