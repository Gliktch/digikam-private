/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "piwigoitem.h"
#include "wstooldialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericPiwigoPlugin
{

class PiwigoWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit PiwigoWindow(DInfoInterface* const iface,
                          QWidget* const parent);
    ~PiwigoWindow() override;

private:

    void connectSignals();
    void readSettings();
    QString cleanName(const QString&) const;
    void setUiInProgressState(bool inProgress);

private Q_SLOTS:

    void slotDoLogin();
    void slotLoginFailed(const QString& msg);
    void slotBusy(bool val);
    void slotProgressInfo(const QString& msg);
    void slotError(const QString& msg);
    void slotAlbums(const QList<PiwigoAlbum>& albumList);
    void slotAlbumSelected();
    void slotAddPhoto();
    void slotAddPhotoNext();
    void slotAddPhotoSucceeded();
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoCancel();
    void slotEnableSpinBox(int n);
    void slotSettings();
    void slotReloadAlbums();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPiwigoPlugin
