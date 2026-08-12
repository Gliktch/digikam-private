/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : A widget to display IPTC metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT IptcWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit IptcWidget(QWidget* const parent,
                        const QString& name = QString());
    ~IptcWidget()                                   override = default;

    bool loadFromURL(const QUrl& url)               override;

    QString getTagDescription(const QString& key)   override;
    QString getTagTitle(const QString& key)         override;

    QString getMetadataTitle() const                override;

protected Q_SLOTS:

    void slotSaveMetadataToFile()                   override;

private:

    bool decodeMetadata()                           override;
    void buildView()                                override;

private:

    QStringList m_keysFilter;
};

} // namespace Digikam
