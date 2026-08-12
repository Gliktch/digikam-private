/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "wssettingswidget.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

class RajceWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit RajceWidget(DInfoInterface* const iface, QWidget* const parent);
    ~RajceWidget() override;

Q_SIGNALS:

    void signalLoginStatusChanged(bool loggedIn);

public Q_SLOTS:

    void slotStartUpload();

public:

    void reactivate();
    void cancelUpload();

    void writeSettings();
    void readSettings();

private Q_SLOTS:

    void slotChangeUserClicked();

    void slotProgressStarted(unsigned);
    void slotProgressFinished(unsigned);
    void slotProgressChanged(unsigned, unsigned percent);

    void slotLoadAlbums();
    void slotCreateAlbum();
    void slotCloseAlbum();

    void slotUploadNext();

    void slotStartUploadAfterAlbumOpened();
    void slotSelectedAlbumChanged(const QString&);

private:

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

    void setEnabledWidgets(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericRajcePlugin
