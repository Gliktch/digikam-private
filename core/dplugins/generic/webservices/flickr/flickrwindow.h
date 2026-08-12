/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QHash>
#include <QList>
#include <QPair>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

// Local includes

#include "wscomboboxintermediate.h"
#include "dinfointerface.h"
#include "wstooldialog.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class FlickrWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit FlickrWindow(DInfoInterface* const iface,
                          QWidget* const parent,
                          const QString& serviceName = QLatin1String("Flickr"));
    ~FlickrWindow()                                                   override;

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotLinkingSucceeded();
    void slotBusy(bool val);
    void slotError(const QString& msg);
    void slotFinished();
    void slotUser1();
    void slotCancelClicked();

    void slotCreateNewPhotoSet();
    void slotUserChangeRequest();
    void slotRemoveAccount();
    void slotPopulatePhotoSetComboBox();
    void slotAddPhotoNext();
    void slotAddPhotoSucceeded(const QString& photoId);
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoSetSucceeded();
    void slotListPhotoSetsFailed(const QString& msg);
    void slotAddPhotoCancelAndClose();
    void slotAuthCancel();
    void slotImageListChanged();
    void slotReloadPhotoSetRequest();

private:

    QString guessSensibleSetName(const QList<QUrl>& urlList)    const;

    void closeEvent(QCloseEvent*)                                     override;
    void readSettings(const QString& uname);
    void writeSettings();

    void setUiInProgressState(bool inProgress);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFlickrPlugin
