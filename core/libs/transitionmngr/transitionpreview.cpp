/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show transition preview
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "transitionpreview.h"

// Qt includes

#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>

// Local includes

#include "frameutils.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN TransitionPreview::Private
{
public:

    Private() = default;

    QTimer                    restartTimer;
    QTimer                    transTimer;
    TransitionMngr*           mngr              = nullptr;
    TransitionMngr::TransType curTransition     = TransitionMngr::None;
    QSize                     previewSize       = QSize(192, 144);
};

TransitionPreview::TransitionPreview(QWidget* const parent)
    : QLabel(parent),
      d     (new Private)
{
    setFixedSize(d->previewSize);
    setContentsMargins(QMargins());
    setScaledContents(false);
    setOpenExternalLinks(false);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    d->mngr = new TransitionMngr;
    d->mngr->setOutputSize(d->previewSize);

    connect(&d->transTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTransition()));

    connect(&d->restartTimer, SIGNAL(timeout()),
            this, SLOT(slotRestart()));
}

TransitionPreview::~TransitionPreview()
{
    delete d;
}

void TransitionPreview::setImagesList(const QList<QUrl>& images)
{
    if (!images.isEmpty())
    {
        d->mngr->setInImage(FrameUtils::makeFramedImage(images[0].toLocalFile(), d->previewSize));

        if (images.count() > 1)
        {
            d->mngr->setOutImage(FrameUtils::makeFramedImage(images[1].toLocalFile(), d->previewSize));
        }
        else
        {
            QImage blank(d->previewSize, QImage::Format_ARGB32);
            blank.fill(Qt::black);
            d->mngr->setOutImage(blank);
        }
    }
    else
    {
        QImage sample = QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                      QLatin1String("digikam/data/sample-aix.webp")));
        d->mngr->setInImage(sample);

        QImage blank(d->previewSize, QImage::Format_ARGB32);
        blank.fill(Qt::black);
        d->mngr->setOutImage(blank);
    }
}

void TransitionPreview::startPreview(TransitionMngr::TransType eff)
{
    stopPreview();
    d->curTransition = eff;
    d->mngr->setTransition(eff);
    d->transTimer.start(100);
}

void TransitionPreview::slotProgressTransition()
{
    int tmout  = -1;
    QImage img = d->mngr->currentFrame(tmout);
    setPixmap(QPixmap::fromImage(img));

    if (tmout == -1)
    {
        stopPreview();
        d->restartTimer.start(1000);
    }
}

void TransitionPreview::stopPreview()
{
    d->transTimer.stop();
    d->restartTimer.stop();
}

void TransitionPreview::slotRestart()
{
    startPreview(d->curTransition);
}

} // namespace Digikam

#include "moc_transitionpreview.cpp"
