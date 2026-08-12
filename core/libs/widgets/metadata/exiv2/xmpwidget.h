/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-19
 * Description : A widget to display XMP metadata
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class DIGIKAM_EXPORT XmpWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit XmpWidget(QWidget* const parent,
                       const QString& name = QString());
    ~XmpWidget()                                    override = default;

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
