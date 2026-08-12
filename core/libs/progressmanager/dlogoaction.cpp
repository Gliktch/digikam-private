/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : a tool bar action object to display animated logo during long operations
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dlogoaction.h"

// Qt includes

#include <QPalette>
#include <QPainter>
#include <QPointer>
#include <QTimer>
#include <QProxyStyle>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "daboutdata.h"

namespace Digikam
{

/**
 * @brief class used to override the widget style to the DLogoAction hosted in a tool bar to prevent
 *        the keyboard shortcut underline in the action text done by the style applied by the application.
 */
class Q_DECL_HIDDEN NoUnderlineToolButtonStyle : public QProxyStyle
{
    Q_OBJECT

public:

    NoUnderlineToolButtonStyle() = default;

    int styleHint(StyleHint hint,
                  const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr,
                  QStyleHintReturn* returnData = nullptr) const override
    {
        if (
            widget &&
            widget->property("noUnderline").toBool() &&
            (hint == QStyle::SH_UnderlineShortcut)
           )
        {
            return 0; // Disable the underline for the keyboard shortcuts.
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }

private:

    // Disable
    NoUnderlineToolButtonStyle(QObject*);
};

// ---

class Q_DECL_HIDDEN DLogoAction::Private
{
public:

    Private() = default;

public:

    int           progressCount   = 0;          ///< Animation position (0 to 35).
    QTimer*       progressTimer   = nullptr;    ///< The timer to manage the animation.
    QIcon         logo;                         ///< Store the original icon.
    int           logoHeight      = 48;         ///< The max size of icon hosted in toolbar.
};

DLogoAction::DLogoAction(QObject* const parent)
    : QAction(parent),
      d      (new Private)
{
    setIconText(QLatin1String("digiKam.org"));
    setToolTip(i18n("Visit digiKam project website"));

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        setIcon(QIcon::fromTheme(QLatin1String("digikam")));
    }
    else
    {
        setIcon(QIcon::fromTheme(QLatin1String("showfoto")));
    }

    d->logo          = icon();
    d->progressTimer = new QTimer(this);
    d->progressTimer->setSingleShot(true);

    connect(d->progressTimer, &QTimer::timeout,
            this, &DLogoAction::slotProgressTimerDone);

    connect(this, &QAction::triggered,
            this, []()
        {
            QDesktopServices::openUrl(DAboutData::webProjectUrl());
        }
    );
}

DLogoAction::~DLogoAction()
{
    delete d;
}

void DLogoAction::start()
{
    d->progressCount = 0;
    d->progressTimer->start(100); // 100 ms between frame
}

void DLogoAction::stop()
{
    d->progressCount = 0;
    d->progressTimer->stop();

    // Reset the display with the static icon

    setIcon(d->logo);
}

bool DLogoAction::running() const
{
    return d->progressTimer->isActive();
}

QPixmap DLogoAction::renderAnimationFrame(int beamPosition)
{
    QSize size = QSize(256, 256);
    QPixmap frame(d->logo.actualSize(size));
    frame.fill(Qt::transparent);

    // Draw the icon

    QPainter painter(&frame);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(QPoint(0, 0), d->logo.pixmap(size));

    // Create a mask for the non-transparent pixels.

    QImage frameImage = frame.toImage();
    QImage mask(frame.size(), QImage::Format_Grayscale8);
    mask.fill(0);                                           // NOTE: black = transparent
    {
        QPainter maskPainter(&mask);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        maskPainter.drawPixmap(QPoint(0, 0), d->logo.pixmap(size));
    }

    // Lightning settings.

    int beamWidth     = frame.height();     // Lightning width (ajustable).
    int halfBeamWidth = beamWidth / 6;      // half width for the symmetrical gradient.

    // Apply the lightning with a symmetrical gradient.

    for (int y = 0 ; y < frame.height() ; ++y)
    {
        for (int x = 0 ; x < frame.width() ; ++x)
        {
            // Check if the pixel is transparent.

            if (qAlpha(frameImage.pixel(x, y)) > 0)
            {
                // Compute the distance with the center of the lightning.

                int distToCenter = abs(x - beamPosition);

                // If the pixel is inside the lightning.

                if (distToCenter < halfBeamWidth)
                {
                    // Compute the intensity (symmetrical gradient).

                    float intensity = 1.0F - (static_cast<float>(distToCenter) / halfBeamWidth);
                    intensity       = qMin(intensity, 1.0f); // Limit to 1.0

                    // Apply the intensity to the pixel.

                    QRgb pixel      = frameImage.pixel(x, y);
                    int r           = qMin(qRed(pixel)   + static_cast<int>(intensity * 200), 255);
                    int g           = qMin(qGreen(pixel) + static_cast<int>(intensity * 200), 255);
                    int b           = qMin(qBlue(pixel)  + static_cast<int>(intensity * 200), 255);
                    frameImage.setPixel(x, y, qRgba(r, g, b, qAlpha(pixel)));
                }
            }
        }
    }

    return QPixmap::fromImage(frameImage);
}

void DLogoAction::slotProgressTimerDone()
{
    int beamPosition = d->progressCount * (d->logoHeight / 5);
    QPixmap frame    = renderAnimationFrame(beamPosition);
    setIcon(frame);

    d->progressCount++;

    if (d->progressCount > d->logoHeight)
    {
        d->progressCount = 0;
    }

    d->progressTimer->start(100);
}

void DLogoAction::applyStyleForToolBarButton(QToolButton* const btn)
{
    btn->setCursor(Qt::PointingHandCursor);
    btn->setAutoRaise(false);
    btn->setStyleSheet(QLatin1String(
        "QToolButton {"
        "    border: none;"
        "    background: transparent;"
        "}"
        "QToolButton:hover {"
        "    background: transparent;"
        "    border: none;"
        "}")
    );

    btn->setProperty("noUnderline", true);         // Mark the button for the underline removing.
    QPointer<NoUnderlineToolButtonStyle> ptr = new NoUnderlineToolButtonStyle;
    btn->setStyle(ptr);                            // Apply the personalised style to remove it.
}

} // namespace Digikam

#include "moc_dlogoaction.cpp"

#include "dlogoaction.moc"
