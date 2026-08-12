/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
#include <QUrl>

// KDE includes

#include <kjob.h>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

namespace MediaWiki
{
    class Iface;
}

using namespace MediaWiki;

namespace DigikamGenericMediaWikiPlugin
{

class MediaWikiTalker : public KJob
{
    Q_OBJECT

public:

    explicit MediaWikiTalker(DInfoInterface* const iface, Iface* const MediaWiki, QObject* const parent = nullptr);
    ~MediaWikiTalker() override;

public:

    QString buildWikiText(const QMap<QString, QString>& info) const;

    void setImageMap(const QMap <QString, QMap <QString, QString> >& imageDesc);
    void start() override;

Q_SIGNALS:

    void signalUploadProgress(int percent);
    void signalEndUpload();

public Q_SLOTS:

    void slotBegin();
    void slotUploadHandle(KJob* j = nullptr);
    void slotUploadProgress(KJob* job, unsigned long percent);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMediaWikiPlugin
