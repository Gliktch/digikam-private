/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC categories settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Victor Dodon <dodonvictor at gmail dot com>
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

class IPTCCategories : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit IPTCCategories(QWidget* const parent);
    ~IPTCCategories() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotCategorySelectionChanged();
    void slotAddCategory();
    void slotDelCategory();
    void slotRepCategory();

    void slotCheckCategoryToggled(bool checked);
    void slotCheckSubCategoryToggled(bool checked);

    void slotLineEditModified();

private:

    void enableWidgets(bool checked1, bool checked2);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
