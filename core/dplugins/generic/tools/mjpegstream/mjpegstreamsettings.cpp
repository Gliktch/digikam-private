/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG stream settings container.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegstreamsettings.h"

// Qt includes

#include <QFontDatabase>

// KDE includes

#include <kconfiggroup.h>

namespace DigikamGenericMjpegStreamPlugin
{

void MjpegStreamSettings::setCollectionMap(const MjpegServerMap& map)
{
    inputImages.clear();

    for (MjpegServerMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        inputImages.append(it.value());
    }
}

void MjpegStreamSettings::readSettings(const KConfigGroup& group)
{
    port                 = group.readEntry("MJPEGStreamPort",                                  8080);
    loop                 = group.readEntry("MJPEGStreamLoop",                                  true);
    quality              = group.readEntry("MJPEGStreamQuality",                               75);
    delay                = group.readEntry("MJPEGStreamDelay",                                 5);
    rate                 = group.readEntry("MJPEGStreamRate",                                  10);
    outSize              = group.readEntry("MJPEGStreamOutSize",                               (int)VidSlideSettings::BLUERAY);
    effect               = (EffectMngr::EffectType)group.readEntry("MJPEGStreamEffect",        (int)EffectMngr::None);
    transition           = (TransitionMngr::TransType)group.readEntry("MJPEGStreamTransition", (int)TransitionMngr::None);

    osdSettings.readSettings(group);
}

void MjpegStreamSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("MJPEGStreamPort",                      port);
    group.writeEntry("MJPEGStreamLoop",                      loop);
    group.writeEntry("MJPEGStreamQuality",                   quality);
    group.writeEntry("MJPEGStreamRate",                      rate);
    group.writeEntry("MJPEGStreamDelay",                     delay);
    group.writeEntry("MJPEGStreamOutSize",                   outSize);
    group.writeEntry("MJPEGStreamEffect",                    (int)effect);
    group.writeEntry("MJPEGStreamTransition",                (int)transition);

    osdSettings.writeSettings(group);
}

} // namespace DigikamGenericMjpegStreamPlugin
