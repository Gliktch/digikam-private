/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display Standard Exif metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit ExifWidget(QWidget* const parent, const QString& name = QString());
    ~ExifWidget()                                   override = default;

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
