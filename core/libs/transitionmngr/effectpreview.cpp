/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show video frame effect preview
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "effectpreview.h"

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

class Q_DECL_HIDDEN EffectPreview::Private
{
public:

    Private() = default;

    QTimer                 restartTimer;
    QTimer                 effTimer;
    EffectMngr*            mngr         = nullptr;
    EffectMngr::EffectType curEffect    = EffectMngr::None;
    QSize                  previewSize  = QSize(192, 144);
};

EffectPreview::EffectPreview(QWidget* const parent)
    : QLabel(parent),
      d     (new Private)
{
    setFixedSize(d->previewSize);
    setContentsMargins(QMargins());
    setScaledContents(false);
    setOpenExternalLinks(false);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    d->mngr = new EffectMngr;
    d->mngr->setOutputSize(d->previewSize);

    connect(&d->effTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressEffect()));

    connect(&d->restartTimer, SIGNAL(timeout()),
            this, SLOT(slotRestart()));
}

EffectPreview::~EffectPreview()
{
    delete d;
}

void EffectPreview::setImagesList(const QList<QUrl>& images)
{
    if (!images.isEmpty())
    {
        d->mngr->setImage(FrameUtils::makeFramedImage(images[0].toLocalFile(), d->previewSize));
    }
    else
    {
        QImage sample = QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                      QLatin1String("digikam/data/sample-aix.webp")));
        d->mngr->setImage(sample.scaled(QSize(1024, 768), Qt::KeepAspectRatio));
    }
}

void EffectPreview::startPreview(EffectMngr::EffectType eff)
{
    stopPreview();
    d->curEffect = eff;
    d->mngr->setEffect(eff);
    d->effTimer.start(50);
}

void EffectPreview::slotProgressEffect()
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

void EffectPreview::stopPreview()
{
    d->effTimer.stop();
    d->restartTimer.stop();
}

void EffectPreview::slotRestart()
{
    startPreview(d->curEffect);
}

} // namespace Digikam

#include "moc_effectpreview.cpp"
