/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class IccProfile;

class DIGIKAM_EXPORT ICCProfileWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit ICCProfileWidget(QWidget* const parent, int w = 256, int h = 256);
    ~ICCProfileWidget()                                   override;

    bool    loadFromURL(const QUrl& url)                  override;
    bool    loadFromProfileData(const QString& fileName,
                                const QByteArray& data);
    bool    loadProfile(const QString& fileName,
                        const IccProfile& data);

    QString getTagDescription(const QString& key)         override;
    QString getTagTitle(const QString& key)               override;

    QString getMetadataTitle()                      const override;

    void    setLoadingFailed();
    void    setDataLoading();
    void    setUncalibratedColor();

    bool  setProfile(const IccProfile& profile);
    IccProfile getProfile()                         const;

protected Q_SLOTS:

    void slotSaveMetadataToFile()                         override;

private:

    bool decodeMetadata()                                 override;
    void buildView()                                      override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
