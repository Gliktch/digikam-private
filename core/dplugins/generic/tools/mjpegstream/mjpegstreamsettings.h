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

#pragma once

// Qt includes

#include <QList>
#include <QString>
#include <QUrl>
#include <QFont>

// Local includes

#include "vidslidesettings.h"
#include "mjpegserver.h"
#include "dinfointerface.h"
#include "effectmngr.h"
#include "transitionmngr.h"
#include "frameosdsettings.h"

class KConfigGroup;

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegStreamSettings
{

public:

    MjpegStreamSettings()  = default;
    ~MjpegStreamSettings() = default;

    /**
     * Helper method to compute urlsList from a set of albms selected by end-users from GUI.
     */
    void setCollectionMap(const MjpegServerMap& map);

    /**
     * Read and write settings in config file between sessions.
     */
    void  readSettings(const KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

public:

    FrameOsdSettings            osdSettings;

    // ---

    int                         port        = 8080;                         ///< IP port to use with MJPEG Server.
    bool                        loop        = true;                         ///< Image stream as loop.
    int                         quality     = 75;                           ///< Jpeg compression [1...100].
    int                         delay       = 5;                            ///< Delay in seconds between inages.
    QList<QUrl>                 inputImages;                                ///< Ordered list of images to stream.
    int                         outSize     = VidSlideSettings::BLUERAY;    ///< Output JPEG size ID.
    int                         rate        = 10;                           ///< Number of frames by seconds.
    TransitionMngr::TransType   transition  = TransitionMngr::None;         ///< Transition type between images.
    EffectMngr::EffectType      effect      = EffectMngr::None;             ///< Effect while displaying images.
    DInfoInterface*             iface       = nullptr;                      ///< Plugin host interface to handle item properties.
};

} // namespace DigikamGenericMjpegStreamPlugin
