/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dsplashscreen.h"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QFont>
#include <QString>
#include <QColor>
#include <QTime>
#include <QTextDocument>
#include <QFontDatabase>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "daboutdata.h"
#include "digikam_version.h"

namespace Digikam
{

class Q_DECL_HIDDEN DSplashScreen::Private
{
public:

    Private() = default;

public:

    int     state               = 0;
    int     progressBarSize     = 3;

    QTimer* timer               = nullptr;

    QString message;
    QString version             = QLatin1String(digikam_version_short);

    QColor  messageColor        = Qt::white;
    QColor  versionColor        = Qt::white;

    QTime   lastStateUpdateTime = QTime::currentTime();
};

DSplashScreen::DSplashScreen()
    : QSplashScreen(QPixmap()),
      d            (new Private)
{
    QPixmap splash;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        splash = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/splash-digikam.webp"));
    }
    else
    {
        splash = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("showfoto/data/splash-showfoto.webp"));
    }

    if (!QString::fromUtf8(digikam_version_suffix).isEmpty())
    {
        QPainter p(&splash);
        p.drawPixmap(380, 27, QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-beta.webp")));
        p.end();
    }

    setPixmap(splash);

    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotAnimate()));

    d->timer->start(150);
}

DSplashScreen::~DSplashScreen()
{
    d->timer->stop();

    delete d;
}

void DSplashScreen::setColor(const QColor& color)
{
    d->messageColor = color;
}

void DSplashScreen::setMessage(const QString& message)
{
    d->message = message;

    showMessage(
                d->message,
                (layoutDirection() == Qt::RightToLeft) ? Qt::AlignLeft : Qt::AlignRight,
                d->messageColor
               );

    slotAnimate();
    qApp->processEvents();
}

void DSplashScreen::slotAnimate()
{
    QTime currentTime = QTime::currentTime();

    if (d->lastStateUpdateTime.msecsTo(currentTime) > 100)
    {
        d->state               = ((d->state + 1) % (2 * d->progressBarSize - 1));
        d->lastStateUpdateTime = currentTime;
    }

    update();
}

void DSplashScreen::drawContents(QPainter* p)
{
    int    position = 0;
    QColor basecolor(155, 192, 231);

    // -- Draw background circles ------------------------------------

    QPainter::RenderHints hints = p->renderHints();
    p->setRenderHints(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(225, 234, 231));
    p->drawEllipse(21, 6, 9, 9);
    p->drawEllipse(32, 6, 9, 9);
    p->drawEllipse(43, 6, 9, 9);
    p->setRenderHints(hints);

    // -- Draw animated circles --------------------------------------

    // Increments are chosen to get close to background's color
    // (didn't work well with QColor::light function)

    for (int i = 0 ; i < d->progressBarSize ; ++i)
    {
        position = (d->state + i) % (2 * d->progressBarSize - 1);

        if (position < 3)
        {
            p->setBrush(QColor(basecolor.red()   - 18*i,
                               basecolor.green() - 28*i,
                               basecolor.blue()  - 10*i));

            p->drawEllipse(21 + position*11, 6, 9, 9);
        }
    }

    // We use a device dependent font with a fixed size.

    QFont fnt(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    fnt.setPixelSize(10);
    fnt.setBold(false);
    p->setFont(fnt);

    QRect r = rect();
    r.setCoords(r.x() + 60, r.y() + 4, r.width() - 10, r.height() - 10);

    // -- Draw message --------------------------------------------------------

    // Message is draw at given position, limited to 49 chars
    // If message is too long, string is truncated

    if (d->message.length() > 50)
    {
        d->message.truncate(49);
    }

    p->setPen(d->messageColor);
    p->drawText(r, d->message);

    // -- Draw version string -------------------------------------------------

    QFontMetrics fontMt(fnt);
    QRect r2 = fontMt.boundingRect(rect(), 0, d->version);
    r2.moveTopLeft(QPoint(width() - r2.width() - 10, r.y()));
    p->setPen(d->versionColor);
    p->drawText(r2, d->version);

    // -- Draw slogan and family ----------------------------------------------

    // NOTE: splashscreen size is 500*307 pixels

    r = rect();
    r.setCoords(r.x() + 207, r.y() + 234, r.x() + 497, r.y() + 277);
    p->translate(r.x(), r.y());
    QTextDocument slogan;
    slogan.setDefaultTextOption(QTextOption(Qt::AlignVCenter));
    slogan.setHtml(DAboutData::digiKamSloganFormated());
    slogan.setPageSize(r.size());
    slogan.setDefaultFont(fnt);
    slogan.drawContents(p, QRect(0, 0, r.width(), r.height()));
}

} // namespace Digikam

#include "moc_dsplashscreen.cpp"
