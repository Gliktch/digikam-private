/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "panooptimizepage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "autooptimiserbinary.h"
#include "panomodifybinary.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoOptimizePage::Private
{
public:

    Private() = default;

public:

    int             progressCount               = 0;
    QLabel*         progressLabel               = nullptr;
    QTimer*         progressTimer               = nullptr;
    QMutex          progressMutex;              ///< This is a precaution in case the user does a back / next action at the wrong moment
    bool            optimisationDone            = false;
    bool            canceled                    = false;

    QLabel*         title                       = nullptr;
/*
    QLabel*         preprocessResults           = nullptr;
*/
    QCheckBox*      horizonCheckbox             = nullptr;
/*
    QCheckBox*      projectionAndSizeCheckboxs  = nullptr;
*/
    QTextBrowser*   detailsText                 = nullptr;

    DWorkingPixmap* progressPix                 = nullptr;

    PanoManager*    mngr                        = nullptr;
};

PanoOptimizePage::PanoOptimizePage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, QString::fromLatin1("<b>%1</b>").arg(i18nc("@title: window", "Optimization"))),
      d          (new Private)
{
    d->mngr                         = mngr;
    d->progressTimer                = new QTimer(this);
    d->progressPix                  = new DWorkingPixmap(this);
    DVBox* const vbox               = new DVBox(this);
    d->title                        = new QLabel(vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    KSharedConfigPtr config         = KSharedConfig::openConfig();
    KConfigGroup group              = config->group(QLatin1String("Panorama Settings"));

    d->horizonCheckbox              = new QCheckBox(i18nc("@option: check", "Level horizon"), vbox);
    d->horizonCheckbox->setChecked(group.readEntry("Horizon", true));
    d->horizonCheckbox->setToolTip(i18nc("@info: tooltip", "Detect the horizon and adapt the project to make it horizontal."));
    d->horizonCheckbox->setWhatsThis(i18nc("@info: whatsthis", "\"Level horizon\": Detect the horizon and adapt the projection so that "
                                           "the detected horizon is an horizontal line in the final panorama"));
/*
    if (!d->mngr->gPano())
    {
        d->projectionAndSizeCheckbox = new QCheckBox(i18nc("@option: check", "Automatic projection and output aspect"), vbox);
        d->projectionAndSizeCheckbox->setChecked(group.readEntry("Output Projection And Size", true));
        d->projectionAndSizeCheckbox->setToolTip(i18nc("@info: tooltip", "Adapt the projection of the panorama and the area rendered on the "
                                                       "resulting projection so that every photo fits in the resulting "
                                                       "panorama."));
        d->projectionAndSizeCheckbox->setWhatsThis(i18nc("@info: whatsthis", "\"Automatic projection and output aspect\": Automatically "
                                                         "adapt the projection and the area rendered of the panorama to "
                                                         "get every photos into the panorama."));
    }
    else
    {
        d->projectionAndSizeCheckbox = new QCheckBox(i18nc("@option: check", "Automatic output aspect"), vbox);
        d->projectionAndSizeCheckbox->setChecked(group.readEntry("Output Projection And Size", true));
        d->projectionAndSizeCheckbox->setToolTip(i18nc("@info: tooltip", "Adapt the area rendered on the resulting projection so that "
                                                       "every photo fits in the resulting panorama."));
        d->projectionAndSizeCheckbox->setWhatsThis(i18nc("@info: whatsthis", "\"Automatic output aspect\": Automatically adapt the area "
                                                         "rendered of the panorama to get every photos into the panorama."));
    }
*/

/*
    d->preprocessResults    = new QLabel(vbox);
*/

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->detailsText          = new QTextBrowser(vbox);
    d->detailsText->hide();

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->progressLabel        = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignCenter);

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-hugin.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

PanoOptimizePage::~PanoOptimizePage()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Panorama Settings"));
    group.writeEntry("Horizon", d->horizonCheckbox->isChecked());
/*
    group.writeEntry("Output Projection And Size", d->projectionAndSizeCheckbox->isChecked());
*/
    config->sync();

    delete d;
}

void PanoOptimizePage::process()
{
    QMutexLocker lock(&d->progressMutex);

    d->title->setText(QString::fromUtf8("<qt>"
                                        "<p>%1</p>"
                                        "<p>%2</p>"
                                        "</qt>")
                      .arg(i18nc("@info", "Optimization is in progress, please wait."))
                      .arg(i18nc("@info", "This can take a while...")));

    d->horizonCheckbox->hide();
/*
    d->projectionAndSizeCheckbox->hide();
*/
    d->progressTimer->start(300);

    connect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    d->mngr->resetAutoOptimisePto();
    d->mngr->resetViewAndCropOptimisePto();
    d->mngr->thread()->optimizeProject(d->mngr->cpCleanPtoUrl(),
                                       d->mngr->autoOptimisePtoUrl(),
                                       d->mngr->viewAndCropOptimisePtoUrl(),
                                       d->horizonCheckbox->isChecked(),
                                       d->mngr->gPano(),
                                       d->mngr->autoOptimiserBinary().path(),
                                       d->mngr->panoModifyBinary().path());
}

void PanoOptimizePage::initializePage()
{
    d->title->setText(QString::fromUtf8("<qt>"
                                        "<p>%1</p>"
                                        "<p>%2</p>"
                                        "<p>%3</p>"
                                        "<p>%4</p>"
                                        "</qt>")
                      .arg(i18nc("@info", "The optimization step according to your settings is ready to be performed."))
                      .arg(i18nc("@info", "This step can include an automatic leveling of the horizon, and also "
                                          "an automatic projection selection and size."))
                      .arg(i18nc("@info", "To perform this operation, the \"%1\" program will be used.",
                                          QDir::toNativeSeparators(d->mngr->autoOptimiserBinary().path())))
                      .arg(i18nc("@info", "Press the \"Next\" button to run the optimization.")));

/*
    QPair<double, int> result = d->mngr->cpFindUrlData().standardDeviation();
    d->preprocessResults->setText(i18n("Alignment error: %1px", result.first / ((double) result.second)));
*/
    d->detailsText->hide();
    d->horizonCheckbox->show();
/*
    d->projectionAndSizeCheckbox->show();
*/
    d->canceled         = false;
    d->optimisationDone = false;

    setComplete(true);

    Q_EMIT completeChanged();
}

bool PanoOptimizePage::validatePage()
{
    if (d->optimisationDone)
    {
        return true;
    }

    setComplete(false);
    process();

    return false;
}

void PanoOptimizePage::cleanupPage()
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

void PanoOptimizePage::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix->frameAt(d->progressCount));

    if (d->progressPix->frameCount())
    {
        d->progressCount = (d->progressCount + 1) % d->progressPix->frameCount();
    }

    d->progressTimer->start(300);
}

void PanoOptimizePage::slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData& ad)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "SlotPanoAction (optimize)";
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
                case PANO_OPTIMIZE:
                case PANO_AUTOCROP:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Job failed (optimize): " << ad.action;

                    if (d->detailsText->isHidden())
                    {
                        d->title->setText(QString::fromUtf8("<qt>"
                                                            "<p>%1</p>"
                                                            "<p>%2</p>"
                                                            "</qt>")
                                                            .arg(i18nc("@info", "Optimization has failed."))
                                                            .arg(i18nc("@info", "See processing messages below.")));
                        d->progressTimer->stop();
                        d->horizonCheckbox->hide();
/*
                        d->projectionAndSizeCheckbox->hide();
*/
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
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (optimize) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_OPTIMIZE:
                {
                    return;
                }

                case PANO_AUTOCROP:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->progressTimer->stop();
                    d->progressLabel->clear();
                    d->optimisationDone = true;

                    Q_EMIT signalOptimized();

                    initializePage();

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (optimize) " << ad.action;
                    break;
                }
            }
        }
    }
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_panooptimizepage.cpp"
