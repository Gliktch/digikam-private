/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPointer>
#include <QUrl>

// Local includes

#include "dplugingeneric.h"
#include "expoblendingactions.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingThread;
class AlignBinary;
class EnfuseBinary;

class ExpoBlendingManager : public QObject
{
    Q_OBJECT

public:

    explicit ExpoBlendingManager(QObject* const parent = nullptr);
    ~ExpoBlendingManager()                            override;

    static QPointer<ExpoBlendingManager> internalPtr;
    static ExpoBlendingManager*          instance();
    static bool                          isCreated();

    bool checkBinaries();

    void setItemsList(const QList<QUrl>& urls);
    QList<QUrl>& itemsList()                    const;

    void setPlugin(DPlugin* const plugin);

    void setPreProcessedMap(const ExpoBlendingItemUrlsMap& urls);
    ExpoBlendingItemUrlsMap& preProcessedMap()  const;

    ExpoBlendingThread* thread()                const;
    AlignBinary&  alignBinary()                 const;
    EnfuseBinary& enfuseBinary()                const;

    void run();

    /**
     * Clean up all temporary files produced so far.
     */
    void cleanUp();

Q_SIGNALS:

    void updateHostApp(const QUrl& url);

private Q_SLOTS:

    void slotStartDialog();
    void slotSetEnfuseVersion(double version);

private:

    void startWizard();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
