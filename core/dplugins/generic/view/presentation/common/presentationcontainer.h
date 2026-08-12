/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-10-02
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QFont>
#include <QUrl>
#include <QMap>

// Local includes

#include "digikam_config.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

class PresentationMainPage;
class PresentationCaptionPage;
class PresentationAdvPage;

#ifdef HAVE_MEDIAPLAYER

class PresentationAudioPage;

#endif

class PresentationContainer
{

public:

    PresentationContainer() = default;

    ~PresentationContainer()
    {
        delete captionFont;
    };

public:

    int                      delayMsMaxValue                = 0;
    int                      delayMsMinValue                = 0;
    int                      delayMsLineStep                = 0;

    QList<QUrl>              urlList;

    PresentationMainPage*    mainPage                       = nullptr;
    PresentationCaptionPage* captionPage                    = nullptr;

#ifdef HAVE_MEDIAPLAYER

    PresentationAudioPage*   soundtrackPage                 = nullptr;

#endif

    PresentationAdvPage*     advancedPage                   = nullptr;

    // Config file data

    /// Main page
    bool                     opengl                         = false;
    bool                     openGlFullScale                = false;
    int                      delay                          = 0;
    bool                     printFileName                  = false;
    bool                     printProgress                  = false;
    bool                     printFileComments              = false;
    bool                     loop                           = false;
    bool                     shuffle                        = false;
    bool                     offAutoDelay                   = false;
    QString                  effectName;
    QString                  effectNameGL;
    QColor                   bgColor                        = QColor(Qt::black);

    /// Captions page
    uint                     commentsFontColor              = 0;
    uint                     commentsBgColor                = 0;
    bool                     commentsDrawOutline            = false;
    uint                     bgOpacity                      = 10;
    int                      commentsLinesLength            = 0;
    QFont*                   captionFont                    = nullptr;

    /// Soundtrack page
    bool                     soundtrackLoop                 = false;
    bool                     soundtrackPlay                 = false;
    bool                     soundtrackRememberPlaylist     = false;
    bool                     soundtrackPlayListNeedsUpdate  = false;
    QUrl                     soundtrackPath;
    QList<QUrl>              soundtrackUrls;

    /// Advanced page
    bool                     useMilliseconds                = false;
    bool                     enableMouseWheel               = false;
    bool                     enableCache                    = false;
    bool                     kbDisableFadeInOut             = false;
    bool                     kbDisableCrossFade             = false;
    bool                     kbEnableSameSpeed              = false;
    uint                     cacheSize                      = 0;

    QWidget*                 display                        = nullptr;
    DInfoInterface*          iface                          = nullptr;

private:

    /// @note disabled
    PresentationContainer(const PresentationContainer&)            = delete;
    PresentationContainer& operator=(const PresentationContainer&) = delete;
};

} // namespace DigikamGenericPresentationPlugin
