/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
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
#include <QTreeWidgetItem>

// Local includes

#include "wscomboboxintermediate.h"
#include "dinfointerface.h"
#include "wstooldialog.h"
#include "inattaxon.h"
#include "inattalker.h"

using namespace Digikam;

namespace DigikamGenericINatPlugin
{

class INatWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit INatWindow(DInfoInterface* const iface,
                        QWidget* const parent,
                        const QString& serviceName = QLatin1String("iNaturalist"));
    ~INatWindow()                                                   override;

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotLinkingSucceeded(const QString& login, const QString& name,
                              const QUrl& iconUrl);
    void slotLinkingFailed(const QString& error);
    void slotLoadUrlSucceeded(const QUrl&, const QByteArray&);
    void slotApiTokenExpires();
    void slotBusy(bool val);
    void slotError(const QString& msg);
    void slotFinished();
    void slotUser1();
    void slotCancelClicked();

    void slotTaxonSelected(const Taxon&, bool fromVision);
    void slotTaxonDeselected();
    void slotComputerVision();
    void slotObservationCreated(const INatTalker::PhotoUploadRequest&);
    void slotPhotoUploaded(const INatTalker::PhotoUploadResult&);
    void cancelUpload(const INatTalker::PhotoUploadRequest&);
    void slotObservationDeleted(int id);
    void slotUserChangeRequest();
    void slotRemoveAccount();
    void slotAuthCancel();
    void slotImageListChanged();
    void slotValueChanged(int);
    void slotClosestChanged(int);
    void slotMoreOptionsButton(bool);
    void slotNearbyPlaces(const QStringList&);
    void slotNearbyObservation(const INatTalker::NearbyObservation&);

private:

    void closeEvent(QCloseEvent*)                               override;
    void readSettings(const QString& uname);
    void writeSettings();

    void setUiInProgressState(bool inProgress);
    void updateProgressBarValue(unsigned);
    void updateProgressBarMaximum(unsigned);
    void switchUser(bool restoreToken = true);
    void saveEditedPlaceName(const QString&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericINatPlugin
