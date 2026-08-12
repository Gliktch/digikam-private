/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmime_p.h"

namespace Digikam
{

SetupMime::SetupMime(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->panel = new QWidget(viewport());
    setWidget(d->panel);
    setWidgetResizable(true);
    setupMime();
}

SetupMime::~SetupMime()
{
    delete d;
}

void SetupMime::slotShowCurrentImageSettings()
{
    QStringList imageList;
    CoreDbAccess().db()->getFilterSettings(&imageList, nullptr, nullptr);
    QString text = i18n("<p>Files with these extensions will be recognized as images "
                        "and included into the database:<br> <code>%1</code></p>",
                        imageList.join(QLatin1Char(' ')));
    QWhatsThis::showText(d->imageFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->imageFileFilterLabel);
}

void SetupMime::slotShowCurrentMovieSettings()
{
    QStringList movieList;
    CoreDbAccess().db()->getFilterSettings(nullptr, &movieList, nullptr);
    QString text = i18n("<p>Files with these extensions will be recognized as video files "
                        "and included into the database:<br> <code>%1</code></p>",
                        movieList.join(QLatin1Char(' ')));
    QWhatsThis::showText(d->movieFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->movieFileFilterLabel);
}

void SetupMime::slotShowCurrentAudioSettings()
{
    QStringList audioList;
    CoreDbAccess().db()->getFilterSettings(nullptr, nullptr, &audioList);
    QString text = i18n("<p>Files with these extensions will be recognized as audio files "
                        "and included into the database:<br> <code>%1</code></p>",
                        audioList.join(QLatin1Char(' ')));
    QWhatsThis::showText(d->audioFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->audioFileFilterLabel);
}

} // namespace Digikam

#include "moc_setupmime.cpp"
