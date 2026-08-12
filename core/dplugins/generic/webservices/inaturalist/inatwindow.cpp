/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inatwindow.h"

// Qt includes

#include <QPushButton>
#include <QProgressDialog>
#include <QPicture>
#include <QPixmap>
#include <QCheckBox>
#include <QStringList>
#include <QSpinBox>
#include <QPointer>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QWindow>
#include <QCloseEvent>
#include <QTimeZone>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dprogresswdg.h"
#include "ditemslist.h"
#include "dmetadata.h"
#include "dtextedit.h"
#include "wsselectuserdlg.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "inatwidget_p.h"
#include "inatbrowserdlg.h"
#include "inatutils.h"

namespace DigikamGenericINatPlugin
{

/**
 * iNaturalist.org allows up to 20 photos for each observation and
 * scales down photos to 2048 pixels.
 */
enum
{
    MAX_OBSERVATION_PHOTOS = 20,
    MAX_DIMENSION          = 2048,
    MAX_EDITED_PLACES      = 5
};

static const QString xmpNameSpaceURI    = QLatin1String("https://inaturalist.org/ns/1.0/");
static const QString xmpNameSpacePrefix = QLatin1String("iNaturalist");

class Q_DECL_HIDDEN INatWindow::Private
{
public:

    Private() = default;

    QString                 serviceName;

    QPushButton*            changeUserButton            = nullptr;
    QLabel*                 accountIcon                 = nullptr;
    QPushButton*            removeAccount               = nullptr;

    QCheckBox*              resizeCheckBox              = nullptr;

    QSpinBox*               dimensionSpinBox            = nullptr;
    QSpinBox*               imageQualitySpinBox         = nullptr;

    // account info

    QString                 username;
    QString                 name;
    QUrl                    iconUrl;
    QTimer                  apiTokenExpiresTimer;
    QLabel*                 userNameDisplayLabel        = nullptr;
    QProgressDialog*        authProgressDlg             = nullptr;

    // identification

    QLabel*                 identificationImage         = nullptr;
    QLabel*                 identificationLabel         = nullptr;
    bool                    identificationFromVision    = false;
    QLabel*                 closestKnownObservation     = nullptr;
    DPlainTextEdit*         observationDescription      = nullptr;
    TaxonEdit*              identificationEdit          = nullptr;
    SuggestTaxonCompletion* taxonPopup                  = nullptr;
    QComboBox*              placesComboBox              = nullptr;

    // additional options

    QPushButton*            moreOptionsButton           = nullptr;
    QWidget*                moreOptionsWidget           = nullptr;
    QSpinBox*               photoMaxTimeDiffSpB         = nullptr;
    QSpinBox*               photoMaxDistanceSpB         = nullptr;
    QSpinBox*               closestObservationMaxSpB    = nullptr;

    INatWidget*             widget                      = nullptr;
    INatTalker*             talker                      = nullptr;

    DItemsList*             imglst                      = nullptr;

    // observation

    Taxon                   identification;
    bool                    latLonValid                 = false;
    double                  latitude                    = 0.0;
    double                  longitude                   = 0.0;
    QDateTime               observationDateTime;

    QList<QString>          editedPlaces;
    bool                    inCancel                    = false;
    bool                    xmpNameSpace                = false;
    WSSelectUserDlg*        selectUser                  = nullptr;
    DInfoInterface*         iface                       = nullptr;
};

INatWindow::INatWindow(DInfoInterface* const iface,
                       QWidget* const /*parent*/,
                       const QString& serviceName)
    : WSToolDialog(nullptr, QString::fromLatin1("%1 Export Dialog").
                   arg(serviceName)),
       d          (new Private)
{
    d->iface       = iface;
    d->serviceName = serviceName;
    setWindowTitle(i18nc("@title:window", "Export as %1 Observation", d->serviceName));
    setModal(false);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QString::fromLatin1("%1 Export Settings").
                                            arg(d->serviceName));

    if (grp.exists())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString::fromLatin1("%1 Export Settings").arg(d->serviceName)
                                         << "exists, deleting it.";
        grp.deleteGroup();
    }

    d->selectUser  = new WSSelectUserDlg(nullptr, serviceName);
    d->widget      = new INatWidget(this, iface, serviceName);

    // Account

    d->userNameDisplayLabel     = d->widget->getUserNameLabel();
    d->changeUserButton         = d->widget->getChangeUserBtn();
    d->removeAccount            = d->widget->d->removeAccount;
    d->accountIcon              = d->widget->d->accountIcon;

    // Options

    d->resizeCheckBox           = d->widget->getResizeCheckBox();
    d->dimensionSpinBox         = d->widget->getDimensionSpB();
    d->imageQualitySpinBox      = d->widget->getImgQualitySpB();

    // Observation & identification

    d->identificationImage      = d->widget->d->identificationImage;
    d->identificationLabel      = d->widget->d->identificationLabel;
    d->closestKnownObservation  = d->widget->d->closestKnownObservation;
    d->observationDescription   = d->widget->d->observationDescription;
    d->identificationEdit       = d->widget->d->identificationEdit;
    d->taxonPopup               = d->widget->d->taxonPopup;
    d->placesComboBox           = d->widget->d->placesComboBox;
    d->moreOptionsButton        = d->widget->d->moreOptionsButton;
    d->moreOptionsWidget        = d->widget->d->moreOptionsWidget;
    d->photoMaxTimeDiffSpB      = d->widget->d->photoMaxTimeDiffSpB;
    d->photoMaxDistanceSpB      = d->widget->d->photoMaxDistanceSpB;
    d->closestObservationMaxSpB = d->widget->d->closestObservationMaxSpB;

    // Image list

    d->imglst                   = d->widget->d->imglst;

    // max dimension supported by iNaturalist.org

    d->dimensionSpinBox->setMaximum(MAX_DIMENSION);
    d->dimensionSpinBox->setValue(MAX_DIMENSION);

    startButton()->setText(i18nc("@action:button", "Start Uploading"));
    startButton()->setToolTip(QString());

    setMainWidget(d->widget);
    d->widget->setMinimumSize(800, 600);

    connect(d->imglst, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->photoMaxTimeDiffSpB, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));

    connect(d->photoMaxDistanceSpB, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));

    connect(d->closestObservationMaxSpB, SIGNAL(valueChanged(int)),
            this, SLOT(slotClosestChanged(int)));

    connect(d->moreOptionsButton, SIGNAL(toggled(bool)),
            this, SLOT(slotMoreOptionsButton(bool)));

    d->apiTokenExpiresTimer.setSingleShot(true);

    connect(&d->apiTokenExpiresTimer, SIGNAL(timeout()),
            this, SLOT(slotApiTokenExpires()));

    // -------------------------------------------------------------------------

    d->talker = new INatTalker(this, serviceName, d->iface);

    d->taxonPopup->setTalker(d->talker);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLinkingSucceeded(QString,QString,QUrl)),
            this, SLOT(slotLinkingSucceeded(QString,QString,QUrl)));

    connect(d->talker, SIGNAL(signalLinkingFailed(QString)),
            this, SLOT(slotLinkingFailed(QString)));

    connect(d->talker, SIGNAL(signalLoadUrlSucceeded(QUrl,QByteArray)),
            this, SLOT(slotLoadUrlSucceeded(QUrl,QByteArray)));

    connect(d->talker, SIGNAL(signalNearbyPlaces(QStringList)),
            this, SLOT(slotNearbyPlaces(QStringList)));

    connect(d->talker, SIGNAL(signalNearbyObservation(INatTalker::NearbyObservation)),
            this, SLOT(slotNearbyObservation(INatTalker::NearbyObservation)));

    connect(d->talker, SIGNAL(signalObservationCreated(INatTalker::PhotoUploadRequest)),
            this, SLOT(slotObservationCreated(INatTalker::PhotoUploadRequest)));

    connect(d->talker, SIGNAL(signalPhotoUploaded(INatTalker::PhotoUploadResult)),
            this, SLOT(slotPhotoUploaded(INatTalker::PhotoUploadResult)));

    connect(d->talker, SIGNAL(signalObservationDeleted(int)),
            this, SLOT(slotObservationDeleted(int)));

    // -------------------------------------------------------------------------

    connect(d->changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->removeAccount, SIGNAL(clicked()),
            this, SLOT(slotRemoveAccount()));

    // -------------------------------------------------------------------------

    d->authProgressDlg = new QProgressDialog(this);
    d->authProgressDlg->setModal(true);
    d->authProgressDlg->setAutoReset(true);
    d->authProgressDlg->setAutoClose(true);
    d->authProgressDlg->setMaximum(0);
    d->authProgressDlg->reset();

    connect(d->authProgressDlg, SIGNAL(canceled()),
            this, SLOT(slotAuthCancel()));

    d->talker->m_authProgressDlg = d->authProgressDlg;

    // -------------------------------------------------------------------------

    connect(this, &QDialog::finished,
            this, &INatWindow::slotFinished);

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    connect(startButton(), &QPushButton::clicked,
            this, &INatWindow::slotUser1);

    connect(d->taxonPopup, SIGNAL(signalTaxonSelected(Taxon,bool)),
            this, SLOT(slotTaxonSelected(Taxon,bool)));

    connect(d->taxonPopup, SIGNAL(signalTaxonDeselected()),
            this, SLOT(slotTaxonDeselected()));

    connect(d->taxonPopup, SIGNAL(signalComputerVision()),
            this, SLOT(slotComputerVision()));

    d->selectUser->reactivate();
    switchUser();
}

INatWindow::~INatWindow()
{
    delete d->selectUser;
    delete d->authProgressDlg;
    delete d->talker;
    delete d->widget;

    if (d->xmpNameSpace)
    {
        DMetadata::unregisterXmpNameSpace(xmpNameSpaceURI);
    }

    delete d;
}

void INatWindow::switchUser(bool restoreToken)
{
    QString               userName = d->username;
    QList<QNetworkCookie> cookies;

    d->apiTokenExpiresTimer.stop();
    d->talker->unLink();
    d->username                    = QString();
    d->name                        = QString();
    d->iconUrl                     = QUrl();
    d->widget->updateLabels(QString());

    // User gets to select a username unless the timer calls us because
    // our token has expired.

    if (restoreToken)
    {
        userName = d->selectUser->getUserName();
    }

    // If we have a username, restore api token and cookies.

    if (
        !userName.isEmpty() &&
        d->talker->restoreApiToken(userName, cookies, restoreToken)
       )
    {
        // Done if api token has been restored, browser is not called anymore.

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Login skipped; restored api_token "
                                            "for user" << userName;
        return;
    }

    // Pass cookies to browser; if "remember me" is checked on iNaturalist
    // website, the browser will re-login for 14 days without user interaction.

    QPointer<INatBrowserDlg> dlg = new INatBrowserDlg(userName, cookies, this);

    connect(dlg, SIGNAL(signalApiToken(QString,QList<QNetworkCookie>)),
            d->talker, SLOT(slotApiToken(QString,QList<QNetworkCookie>)));

    (void)dlg->exec();
}

void INatWindow::slotApiTokenExpires()
{
    switchUser(false);
}

void INatWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void INatWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void INatWindow::slotFinished()
{
    writeSettings();
    d->imglst->listView()->clear();
}

void INatWindow::setUiInProgressState(bool inProgress)
{
    setRejectButtonMode(inProgress ? QDialogButtonBox::Cancel
                                   : QDialogButtonBox::Close);

    if (inProgress)
    {
        d->widget->progressBar()->show();
    }
    else
    {
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
    }
}

void INatWindow::slotCancelClicked()
{
    if (d->talker->stillUploading())
    {
        d->inCancel = true;
        slotBusy(true);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cancel clicked; deleting "
                                            "observation(s) being uploaded.";
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cancel clicked; not uploading.";

    d->talker->cancel();
    setUiInProgressState(false);
}

void INatWindow::reactivate()
{
    d->userNameDisplayLabel->setText(QString());
    switchUser();

    d->widget->d->imglst->loadImagesFromCurrentSelection();
    show();
}

static const char SETTING_RESIZE[]    = "Resize";
static const char SETTING_DIM[]       = "Maximum Width";
static const char SETTING_QUALITY[]   = "Image Quality";
static const char SETTING_INAT_IDS[]  = "Write iNat Ids";
static const char SETTING_TIME_DIFF[] = "Max Time Diff";
static const char SETTING_DISTANCE[]  = "Max Distance";
static const char SETTING_CLOSEST[]   = "Closest Observation";
static const char SETTING_EXTENDED[]  = "Extended Options";

void INatWindow::readSettings(const QString& uname)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1 %2 Export Settings").
                              arg(d->serviceName, uname);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Group name is:" << groupName;
    KConfigGroup grp        = config->group(groupName);

    d->resizeCheckBox->setChecked(grp.readEntry(SETTING_RESIZE, true));
    d->dimensionSpinBox->setValue(grp.readEntry(SETTING_DIM, 2048));
    d->imageQualitySpinBox->setValue(grp.readEntry(SETTING_QUALITY, 90));
    d->widget->getPhotoIdCheckBox()->setChecked(grp.readEntry(SETTING_INAT_IDS, false));
    d->photoMaxTimeDiffSpB->setValue(grp.readEntry(SETTING_TIME_DIFF, 5));
    d->photoMaxDistanceSpB->setValue(grp.readEntry(SETTING_DISTANCE, 15));
    d->closestObservationMaxSpB->setValue(grp.readEntry(SETTING_CLOSEST, 500));
    d->moreOptionsButton->setChecked(grp.readEntry(SETTING_EXTENDED, false));
    slotMoreOptionsButton(d->moreOptionsButton->isChecked());
}

void INatWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1 %2 Export Settings").arg(d->serviceName, d->username);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Group name is:" << groupName;

    if (QString::compare(QString::fromLatin1("%1 Export Settings").arg(d->serviceName), groupName) == 0)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Not writing entry of group" << groupName;
        return;
    }

    KConfigGroup grp = config->group(groupName);
    grp.writeEntry("username",        d->username);
    grp.writeEntry(SETTING_RESIZE,    d->resizeCheckBox->isChecked());
    grp.writeEntry(SETTING_DIM,       d->dimensionSpinBox->value());
    grp.writeEntry(SETTING_QUALITY,   d->imageQualitySpinBox->value());
    grp.writeEntry(SETTING_INAT_IDS,  d->widget->getPhotoIdCheckBox()->isChecked());
    grp.writeEntry(SETTING_TIME_DIFF, d->photoMaxTimeDiffSpB->value());
    grp.writeEntry(SETTING_DISTANCE,  d->photoMaxDistanceSpB->value());
    grp.writeEntry(SETTING_CLOSEST,   d->closestObservationMaxSpB->value());
    grp.writeEntry(SETTING_EXTENDED,  d->moreOptionsButton->isChecked());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Entry of group" << groupName << "written";
}

void INatWindow::slotMoreOptionsButton(bool setting)
{
    if (setting)
    {
        d->moreOptionsButton->setText(i18n("Fewer options"));
        d->observationDescription->show();
        d->moreOptionsWidget->show();
    }
    else
    {
        d->moreOptionsButton->setText(i18n("More options"));
        d->observationDescription->hide();
        d->moreOptionsWidget->hide();
    }
}

void INatWindow::slotLinkingSucceeded(const QString& username,
                                      const QString& name, const QUrl& iconUrl)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking succeeded for user"
                                     << username;
    d->username = username;
    d->name     = name;
    d->iconUrl  = iconUrl;
    d->apiTokenExpiresTimer.start(1000 * std::max(d->talker->apiTokenExpiresIn(), 1));

    if (!d->name.isEmpty() && (d->name != d->username))
    {
        d->userNameDisplayLabel->setText(QString::fromLatin1("<b>%1 (%2)</b>").
                                         arg(d->username, d->name));
    }
    else
    {
        d->userNameDisplayLabel->setText(QString::fromLatin1("<b>%1</b>").
                                         arg(d->username));
    }

    d->widget->updateLabels(username);
    KSharedConfigPtr config = KSharedConfig::openConfig();
    const auto grps         = config->groupList();

    for (const QString& group : grps)
    {
        if (!(group.contains(d->serviceName)))
        {
            continue;
        }

        KConfigGroup grp = config->group(group);

        if (group.contains(d->username))
        {
            readSettings(d->username);
            break;
        }
    }

    writeSettings();

    if (!d->iconUrl.isEmpty())
    {
        d->talker->loadUrl(d->iconUrl);
    }
}

void INatWindow::slotLinkingFailed(const QString& error)
{
    d->apiTokenExpiresTimer.stop();
    d->accountIcon->hide();
    d->userNameDisplayLabel->setText(i18n("<i>login <b>failed</b></i>"));
    d->widget->updateLabels(QString());
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking failed with error" << error;
}

void INatWindow::slotBusy(bool val)
{
    setCursor(val ? Qt::WaitCursor : Qt::ArrowCursor);
}

void INatWindow::slotError(const QString& msg)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error" << msg;
    QMessageBox::critical(this, i18nc("@title:window", "Error"), msg);
}

void INatWindow::slotUserChangeRequest()
{
    d->apiTokenExpiresTimer.stop();
    writeSettings();
    d->userNameDisplayLabel->setText(QString());
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot Change User Request";
    d->selectUser->reactivate();
    switchUser();
}

void INatWindow::slotRemoveAccount()
{
    d->apiTokenExpiresTimer.stop();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Removing user" << d->username;

    if (d->username.isEmpty())
    {
        return;
    }

    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1 %2 Export Settings").arg(d->serviceName, d->username);
    KConfigGroup grp        = config->group(groupName);

    if (grp.exists())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Removing Account having group" << groupName;
        grp.deleteGroup();
    }

    d->talker->unLink();
    d->talker->removeUserName(d->serviceName + d->username);

    d->accountIcon->hide();
    d->userNameDisplayLabel->setText(QString());
    d->username = QString();
    d->name     = QString();
    d->iconUrl  = QUrl();
}

void INatWindow::slotAuthCancel()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Authorization canceled.";
    d->apiTokenExpiresTimer.stop();
    d->talker->cancel();
    d->authProgressDlg->hide();
    d->accountIcon->hide();
    d->userNameDisplayLabel->setText(i18n("<i>login <b>canceled</b></i>"));
}

void INatWindow::slotTaxonSelected(const Taxon& taxon, bool fromVision)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Taxon" << taxon.name() << "selected"
                                     << (fromVision ? "from vision."
                                                    : "from auto-completion.");

    if (d->identification != taxon)
    {
        d->identification = taxon;
        QString name      = QLatin1String("<h3>") + taxon.htmlName();

        if (!taxon.commonName().isEmpty())
        {
            name += QLatin1String(" (") + taxon.commonName() +
                    QLatin1String(")");
        }

        name += QLatin1String("</h3>");
        d->identificationLabel->setText(name);
        d->talker->loadUrl(taxon.squareUrl());

        startButton()->setEnabled(d->observationDateTime.isValid() &&
                                  d->latLonValid && !d->inCancel &&
                                  (d->imglst->imageUrls().count() <= MAX_OBSERVATION_PHOTOS));

        if (d->latLonValid)
        {
            d->talker->closestObservation(taxon.id(),
                                          d->latitude, d->longitude);
        }
    }

    d->identificationFromVision = fromVision;
}

void INatWindow::slotComputerVision()
{
    const QList<QUrl>& imageUrls = d->imglst->imageUrls();

    if (imageUrls.count())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting computer-vision id for"
                                         << imageUrls[0].toLocalFile();
        d->talker->computerVision(imageUrls[0]);
    }
}

void INatWindow::slotTaxonDeselected()
{
    if (d->identification != Taxon())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Taxon deselected.";
        d->identificationFromVision = false;
        d->identification           = Taxon();
        d->identificationLabel->setText(i18n("<i>no valid identification</i>"));
        d->identificationImage->hide();
        slotNearbyObservation(INatTalker::NearbyObservation());
        startButton()->setEnabled(false);
    }
}

void INatWindow::slotLoadUrlSucceeded(const QUrl& url, const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Image" << url << "received.";

    if      (url == d->identification.squareUrl())
    {
        QImage image;
        image.loadFromData(data);
        d->identificationImage->setPixmap(QPixmap::fromImage(image));
        d->identificationImage->show();
    }
    else if (url == d->iconUrl)
    {
        QImage image;
        image.loadFromData(data);
        d->accountIcon->setPixmap(QPixmap::fromImage(image));
        d->accountIcon->show();
    }
}

void INatWindow::updateProgressBarValue(unsigned diff)
{
    int value = d->widget->progressBar()->value();
    value    += diff;
    d->widget->progressBar()->setValue(value);

    if (value == d->widget->progressBar()->maximum())
    {
        d->widget->progressBar()->reset();
        setUiInProgressState(false);
    }
}

void INatWindow::updateProgressBarMaximum(unsigned diff)
{
    if (d->widget->progressBar()->isHidden())
    {
        d->widget->progressBar()->setMaximum(diff);
        d->widget->progressBar()->setValue(0);
        setUiInProgressState(true);
        d->widget->progressBar()->progressScheduled(i18n("iNaturalist Export"), true, true);
        d->widget->progressBar()->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("dk-inat")).pixmap(22, 22));
    }
    else
    {
        int maximum = d->widget->progressBar()->maximum();
        d->widget->progressBar()->setMaximum(maximum + diff);
    }
}

/**
 * This slot is called when 'Start Uploading' button is pressed.
 */
void INatWindow::slotUser1()
{
    if (
        d->imglst->imageUrls().isEmpty()                          ||
        !d->latLonValid                                           ||
        d->inCancel                                               ||
        (d->imglst->imageUrls().count() > MAX_OBSERVATION_PHOTOS) ||
        !d->observationDateTime.isValid()                         ||
        !d->identification.isValid()
       )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "NOT uploading observation.";
        return;
    }

    startButton()->setEnabled(false);

    // Create an observation.

    QString obsDateTime = d->observationDateTime.toString(Qt::ISODate);
    QJsonObject params;
    params.insert(QLatin1String("observed_on_string"), QJsonValue(obsDateTime));
    params.insert(QLatin1String("time_zone"),
                  QJsonValue(QLatin1String(QTimeZone::systemTimeZoneId())));
    params.insert(QLatin1String("latitude"), QJsonValue(d->latitude));
    params.insert(QLatin1String("longitude"),QJsonValue(d->longitude));
    params.insert(QLatin1String("taxon_id"),
                  QJsonValue(d->identification.id()));

    QString description = d->observationDescription->toPlainText().trimmed();

    if (!description.isEmpty())
    {
        params.insert(QLatin1String("description"), QJsonValue(description));
    }

    QString placeName = d->placesComboBox->currentText().simplified();

    if (placeName != d->placesComboBox->currentText())
    {
        d->placesComboBox->setEditText(placeName);
    }

    if (!placeName.isEmpty())
    {
        params.insert(QLatin1String("place_guess"), QJsonValue(placeName));
        saveEditedPlaceName(placeName);
    }

    params.insert(QLatin1String("owners_identification_from_vision"),
                  QJsonValue(d->identificationFromVision));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Creating observation of"
                                     << d->identification.name() << "from"
                                     << obsDateTime << "with"
                                     << d->imglst->imageUrls().count()
                                     << ((d->imglst->imageUrls().count() == 1) ? "picture."
                                                                               : "pictures.");
    QJsonObject jsonObservation;
    jsonObservation.insert(QLatin1String("observation"), QJsonValue(params));
    updateProgressBarMaximum(1 + d->imglst->imageUrls().count());

    INatTalker::PhotoUploadRequest request(d->imglst->imageUrls(),
                                           d->widget->getPhotoIdCheckBox()->
                                           isChecked(),
                                           d->resizeCheckBox->isChecked(),
                                           d->dimensionSpinBox->value(),
                                           d->imageQualitySpinBox->value(),
                                           d->username);
    d->talker->createObservation(QJsonDocument(jsonObservation).toJson(),
                                 request);

    // Clear data, user can start working on the next observation right away.

    d->imglst->listView()->clear();
    slotTaxonDeselected();
    d->identificationEdit->clear();
    d->observationDescription->clear();
}

void INatWindow::cancelUpload(const INatTalker::PhotoUploadRequest& request)
{
    updateProgressBarMaximum(1);
    updateProgressBarValue(request.m_images.count());
    d->talker->deleteObservation(request.m_observationId, request.m_apiKey);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload canceled, deleting observation"
                                     << request.m_observationId;
}

void INatWindow::slotObservationCreated(const INatTalker::PhotoUploadRequest&
                                        request)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation"
                                     << request.m_observationId
                                     << "created, uploading first picture.";
    updateProgressBarValue(1);

    if      (d->inCancel)
    {
        cancelUpload(request);
    }
    else if (!request.m_images.isEmpty())
    {
        d->talker->uploadNextPhoto(request);
    }
}

void INatWindow::slotPhotoUploaded(const INatTalker::PhotoUploadResult& result)
{
    updateProgressBarValue(1);

    INatTalker::PhotoUploadRequest request(result.m_request);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photo"
                                     << request.m_images.front().toLocalFile()
                                     << "uploaded, observation"
                                     << request.m_observationId
                                     << "observation photo"
                                     << result.m_observationPhotoId
                                     << "photo" << result.m_photoId;

    if (d->inCancel)
    {
        request.m_images.pop_front();
        cancelUpload(request);
        return;
    }

    if (request.m_updateIds)
    {
        const QUrl& fileUrl = request.m_images.front();
        QScopedPointer<DMetadata> meta(new DMetadata);

        if (
            meta->supportXmp()                       &&
            meta->canWriteXmp(fileUrl.toLocalFile()) &&
            meta->load(fileUrl.toLocalFile())
           )
        {
            if (!d->xmpNameSpace)
            {
                meta->registerXmpNameSpace(xmpNameSpaceURI, xmpNameSpacePrefix);
                d->xmpNameSpace = true;
            }

            meta->setXmpTagString("Xmp.iNaturalist.observation",
                                 QString::number(request.m_observationId));
            meta->setXmpTagString("Xmp.iNaturalist.observationPhoto",
                                 QString::number(result.m_observationPhotoId));
            meta->setXmpTagString("Xmp.iNaturalist.photo",
                                 QString::number(result.m_photoId));
            meta->save(fileUrl.toLocalFile());
        }
    }

    request.m_images.pop_front();

    if (!request.m_images.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG)
                << "Uploading next photo" << request.m_images.front().toLocalFile()
                << "to observation" << request.m_observationId;
        d->talker->uploadNextPhoto(request);
    }
}

void INatWindow::slotObservationDeleted(int observationId)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation" << observationId
                                     << "deleted.";
    updateProgressBarValue(1);

    if (!d->talker->stillUploading())
    {
        d->inCancel = false;
        slotBusy(false);
    }
}

void INatWindow::slotNearbyPlaces(const QStringList& places)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received" << places.count() <<
        "nearby places," << d->editedPlaces.count() << "edited places.";

    QString selected = d->placesComboBox->currentText();
    d->placesComboBox->clear();

    for (auto& place : d->editedPlaces)
    {
        d->placesComboBox->addItem(place);

        if (place == selected)
        {
            // Keep previous selection if it is still an option.

            d->placesComboBox->setCurrentText(selected);
        }
    }

    for (auto& place : places)
    {
        d->placesComboBox->addItem(place);

        if (place == selected)
        {
            // Keep previous selection if it is still an option.

            d->placesComboBox->setCurrentText(selected);
        }
    }
}

void INatWindow::slotNearbyObservation(const INatTalker::NearbyObservation&
                                       nearbyObservation)
{
    if (!nearbyObservation.isValid())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "No valid nearby observation.";

        d->closestKnownObservation->clear();
        d->closestKnownObservation->hide();

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received nearby observation.";

    QString red1;
    QString red2;

    if (nearbyObservation.m_distanceMeters > d->closestObservationMaxSpB->value())
    {
        red1 = QLatin1String("<font color=\"red\">");
        red2 = QLatin1String("</font>");
    }

    QString distance(red1 + localizedDistance(nearbyObservation.
                     m_distanceMeters, 'f', 1) + red2);

    QString observation(QString(QLatin1String("<a href=\"https://www.inatura"
                                              "list.org/observations/%1\">")).
                        arg(nearbyObservation.m_observationId) +
                        i18n("observation") + QLatin1String("</a>"));

    QString obscured;

    if (nearbyObservation.m_obscured)
    {
        obscured = QLatin1String("<em>") + i18nc("location", "obscured") +
                   QLatin1String("</em> ");
    }

    QString text(i18n("Closest %1research-grade %2 reported in %3.",
                      obscured, observation, distance));
    d->closestKnownObservation->setText(text);
    d->closestKnownObservation->show();
}

void INatWindow::slotImageListChanged()
{
    static const QLatin1Char lf('\n');

    // number of digits printed for latitude and longitude

    enum
    {
        COORD_PRECISION = 5
    };

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updating image list.";

    bool      latLonValid = false;
    double    latitude    = 0.0;
    double    longitude   = 0.0;
    QDateTime observationTime;

    DItemsListView* const listView = d->widget->d->imglst->listView();

    for (auto& url : d->imglst->imageUrls())
    {
        if (url.isEmpty())
        {
            continue;
        }

        DItemInfo info(d->iface->itemInfo(url));
        QColor txtColor(listView->palette().color(QPalette::Text));
        DItemsListViewItem* const item = listView->findItem(url);
        QDateTime dateTime             = asDateTimeLocal(info.dateTime());
        QString dt;

        if (dateTime.isValid())
        {
            // show date and time of photo, our observation

            if (!observationTime.isValid())
            {
                observationTime = dateTime;
                dt              = QLocale().toString(dateTime, QLocale::ShortFormat) + lf +
                                  i18n("observation time");
                item->setForeground(ItemDate, QBrush(txtColor));
            }
            else
            {
                // show time difference from observation

                int difference = int(qAbs(dateTime.secsTo(observationTime)));
                dt             = localizedTimeDifference(difference) + lf +
                                 i18n("from observation");
                QBrush brush((difference > 60 * d->photoMaxTimeDiffSpB->value()) ? Qt::red : txtColor);
                item->setForeground(ItemDate, brush);
            }
        }
        else
        {
            dt         = i18n("not available");
            QFont font = item->font(ItemDate);
            font.setItalic(true);
            item->setFont(ItemDate, font);
        }

        item->setText(ItemDate, dt);

        QString gps;

        if (info.hasGeolocationInfo())
        {
            if (latLonValid)
            {
                // show distance from observation coordinates

                double distance = distanceBetween(latitude, longitude,
                                                  info.latitude(),
                                                  info.longitude());
                gps             = localizedDistance(distance,'f', 0) + lf +
                                  i18n("from observation");
                QBrush brush((distance > d->photoMaxDistanceSpB->value()) ? Qt::red : txtColor);
                item->setForeground(ItemLocation, brush);
            }
            else
            {
                // show gps coordinates of photo, our observation coordinates

                latLonValid = true;
                latitude    = info.latitude();
                longitude   = info.longitude();
                gps         = localizedLocation(latitude, longitude, COORD_PRECISION) +
                              lf + i18n("observation location");
                item->setForeground(ItemLocation, QBrush(txtColor));
            }
        }
        else
        {
            // gps coordinates not available

            gps        = i18n("not available");
            QFont font = item->font(ItemLocation);
            font.setItalic(true);
            item->setFont(ItemLocation, font);
        }

        item->setText(ItemLocation, gps);
    }

    if (
        (d->latLonValid != latLonValid) ||
        (d->latitude != latitude)       ||
        (d->longitude != longitude)
       )
    {
        if (latLonValid)
        {
            d->talker->nearbyPlaces(latitude, longitude);
        }
        else
        {
            slotNearbyPlaces(QStringList());
        }
    }

    d->latLonValid         = latLonValid;
    d->latitude            = latitude;
    d->longitude           = longitude;
    d->observationDateTime = observationTime;

    startButton()->setEnabled(observationTime.isValid() && latLonValid &&
                              d->identification.isValid() && !d->inCancel &&
                              (d->imglst->imageUrls().count() <= MAX_OBSERVATION_PHOTOS));
}

void INatWindow::slotValueChanged(int)
{
    // Called upon change to d->photoMaxDistanceSpB or d->photoMaxTimeDiffSpB.

    slotImageListChanged();
}

void INatWindow::slotClosestChanged(int)
{
    // Called upon change to d->closestObservationMaxSpB

    if (d->latLonValid && d->identification.isValid())
    {
        d->talker->closestObservation(d->identification.id(),
                                      d->latitude, d->longitude);
    }
    else
    {
        d->closestKnownObservation->clear();
    }
}

void INatWindow::saveEditedPlaceName(const QString& text)
{
    if (!d->editedPlaces.contains(text))
    {
        for (int i = 0; i < d->placesComboBox->count(); ++i)
        {
            if (d->placesComboBox->itemText(i) == text)
            {
                return;
            }
        }
    }

    d->editedPlaces.removeOne(text);
    d->editedPlaces.push_front(text);

    if (d->editedPlaces.count() > MAX_EDITED_PLACES)
    {
        d->editedPlaces.removeLast();
    }
}

} // namespace DigikamGenericINatPlugin

#include "moc_inatwindow.cpp"
