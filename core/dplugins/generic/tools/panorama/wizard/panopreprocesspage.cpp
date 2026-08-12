/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "panopreprocesspage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QPushButton>
#include <QCheckBox>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QList>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "cpcleanbinary.h"
#include "cpfindbinary.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoPreProcessPage::Private
{
public:

    Private() = default;

public:

    int             progressCount           = 0;
    QLabel*         progressLabel           = nullptr;
    QTimer*         progressTimer           = nullptr;
    QMutex          progressMutex;          ///< This is a precaution in case the user does a back / next action at the wrong moment
    bool            preprocessingDone       = false;
    bool            canceled                = false;

    int             nbFilesProcessed        = 0;
    QMutex          nbFilesProcessed_mutex;

    QLabel*         title                   = nullptr;

    QCheckBox*      celesteCheckBox         = nullptr;

    QTextBrowser*   detailsText             = nullptr;

    DWorkingPixmap* progressPix             = nullptr;

    PanoManager*    mngr                    = nullptr;
};

PanoPreProcessPage::PanoPreProcessPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, QString::fromLatin1("<b>%1</b>").arg(i18nc("@title: window", "Pre-Processing Images"))),
      d          (new Private)
{
    d->mngr                 = mngr;
    d->progressTimer        = new QTimer(this);
    d->progressPix          = new DWorkingPixmap(this);
    DVBox* const vbox       = new DVBox(this);
    d->title                = new QLabel(vbox);
    d->title->setWordWrap(true);
    d->title->setOpenExternalLinks(true);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Panorama Settings"));

    d->celesteCheckBox      = new QCheckBox(i18nc("@option: check", "Detect moving skies"), vbox);
    d->celesteCheckBox->setChecked(group.readEntry("Celeste", false));
    d->celesteCheckBox->setToolTip(i18nc("@info: tooltip", "Automatic detection of clouds to prevent wrong keypoints matching "
                                         "between images due to moving clouds."));
    d->celesteCheckBox->setWhatsThis(i18nc("@info: whatsthis", "\"Detect moving skies\": During the control points selection and matching, "
                                           "this option discards any points that are associated to a possible cloud. This "
                                           "is useful to prevent moving clouds from altering the control points matching "
                                           "process."));
    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->detailsText          = new QTextBrowser(vbox);
    d->detailsText->hide();

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->progressLabel        = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignCenter);

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-preprocessing.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

PanoPreProcessPage::~PanoPreProcessPage()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Panorama Settings"));
    group.writeEntry("Celeste", d->celesteCheckBox->isChecked());
    config->sync();

    delete d;
}

void PanoPreProcessPage::process()
{
    QMutexLocker lock(&d->progressMutex);

    d->title->setText(QString::fromUtf8("<qt>"
                                        "<p>%1</p>"
                                        "<p>%2</p>"
                                        "</qt>")
                                        .arg(i18nc("@info", "Pre-processing is in progress, please wait."))
                                        .arg(i18nc("@info", "This can take a while...")));

    d->celesteCheckBox->hide();
    d->progressTimer->start(300);

    connect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));
/*
    d->nbFilesProcessed = 0;
*/
    d->mngr->resetBasePto();
    d->mngr->resetCpFindPto();
    d->mngr->resetCpCleanPto();
    d->mngr->preProcessedMap().clear();
    d->mngr->thread()->preProcessFiles(d->mngr->itemsList(),
                                       d->mngr->preProcessedMap(),
                                       d->mngr->basePtoUrl(),
                                       d->mngr->cpFindPtoUrl(),
                                       d->mngr->cpCleanPtoUrl(),
                                       d->celesteCheckBox->isChecked(),
/*
                                       d->mngr->hdr(),
*/
                                       d->mngr->format(),
                                       d->mngr->gPano(),
                                       d->mngr->cpFindBinary().version(),
                                       d->mngr->cpCleanBinary().path(),
                                       d->mngr->cpFindBinary().path());
}

void PanoPreProcessPage::initializePage()
{
    d->title->setText(QString::fromUtf8("<qt>"
                                        "<p>%1</p>"
                                        "<p>%2</p>"
                                        "<p>%3</p>"
                                        "<p>%4</p>"
                                        "</qt>")
                      .arg(i18nc("@info", "Now, we will pre-process images before stitching them."))
                      .arg(i18nc("@info", "Pre-processing operations include Raw demosaicing. Raw images will be converted "
                                          "to 16-bit sRGB images with auto-gamma."))
                      .arg(i18nc("@info", "Pre-processing also includes a calculation of some control points to match "
                                          "overlaps between images. For that purpose, the \"%1\" program will be used.",
                                          QDir::toNativeSeparators(d->mngr->cpFindBinary().path())))
                      .arg(i18nc("@info", "Press the \"Next\" button to start pre-processing.")));

    d->detailsText->hide();
    d->celesteCheckBox->show();

    d->canceled          = false;
    d->preprocessingDone = false;

    setComplete(true);

    Q_EMIT completeChanged();
}

bool PanoPreProcessPage::validatePage()
{
    if (d->preprocessingDone)
    {
        return true;
    }

    setComplete(false);
    process();

    return false;
}

void PanoPreProcessPage::cleanupPage()
{
    d->canceled = true;

    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    d->mngr->thread()->cancel();

    QMutexLocker lock(&d->progressMutex);

    if (d->progressTimer->isActive())
    {
        d->progressTimer->stop();
        d->progressLabel->clear();
    }
}

void PanoPreProcessPage::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix->frameAt(d->progressCount));

    if (d->progressPix->frameCount())
    {
        d->progressCount = (d->progressCount + 1) % d->progressPix->frameCount();
    }

    d->progressTimer->start(300);
}

void PanoPreProcessPage::slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData& ad)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "SlotPanoAction (preprocessing)";
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "starting, success, canceled, action: " << ad.starting << ad.success << d->canceled << ad.action;

    QMutexLocker lock(&d->progressMutex);

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            if (d->canceled)    // In that case, the error is expected
            {
                return;
            }

            switch (ad.action)
            {
                case PANO_PREPROCESS_INPUT:
                case PANO_CREATEPTO:
                case PANO_CPFIND:
                case PANO_CPCLEAN:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Job failed (preprocessing): " << ad.action;

                    if (d->detailsText->isHidden())  // Ensures only the first failed task is shown
                    {
                        d->title->setText(QString::fromUtf8("<qt>"
                                                            "<p>%1</p>"
                                                            "<p>%2</p>"
                                                            "</qt>")
                                                            .arg(i18nc("@info", "Pre-processing has failed."))
                                                            .arg(i18nc("@info", "See processing messages below.")));

                        d->progressTimer->stop();
                        d->celesteCheckBox->hide();
                        d->detailsText->show();
                        d->progressLabel->clear();
                        d->detailsText->setText(ad.message);

                        setComplete(false);

                        Q_EMIT completeChanged();
                    }

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (preprocessing) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_PREPROCESS_INPUT:
                {
/*
                    QMutexLocker nbProcessed(&d->nbFilesProcessed_mutex);
                    d->nbFilesProcessed++;
*/
                    break;
                }

                case PANO_CREATEPTO:
                case PANO_CPFIND:
                {
                    // Nothing to do, that just another step towards the end
                    break;
                }

                case PANO_CPCLEAN:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->progressTimer->stop();
                    d->progressLabel->clear();
                    d->preprocessingDone = true;

                    Q_EMIT signalPreProcessed();

                    initializePage();

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (preprocessing) " << ad.action;

                    break;
                }
            }
        }
    }
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_panopreprocesspage.cpp"
