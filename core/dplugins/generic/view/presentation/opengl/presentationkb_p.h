/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on
 * smoothslidesaver by Carsten Weinhold <carsten dot weinhold at gmx dot de>
 * and slideshowgl by Renchi Raju <renchi dot raju at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "presentationkb.h"

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationcontainer.h"
#include "kbimageloader.h"
#include "kbeffect.h"

// OpenGL headers is not included automatically with ARM targets

#ifdef Q_PROCESSOR_ARM
#   include <GL/gl.h>
#endif

#ifdef HAVE_MEDIAPLAYER
#   include "presentationaudiowidget.h"
#endif

using namespace Digikam;

namespace DigikamGenericPresentationPlugin
{

class KBImageLoader;
class ScreenProperties;
class PresentationAudioWidget;

class Q_DECL_HIDDEN PresentationKB::Private
{

public:

    Private() = default;

    int                      deskX                  = 0;
    int                      deskY                  = 0;
    int                      deskWidth              = 0;
    int                      deskHeight             = 0;

    QOpenGLTexture*          endTexture             = nullptr;
    KBImageLoader*           imageLoadThread        = nullptr;
    QTimer*                  mouseMoveTimer         = nullptr;
    QTimer*                  timer                  = nullptr;
    bool                     haveImages             = true;

    KBImage*                 image[2]               = { nullptr };
    KBEffect*                effect                 = nullptr;
    int                      numKBEffectRepeated    = 0;
    bool                     zoomIn                 = (QRandomGenerator::global()->bounded(2U) == 0);
    bool                     initialized            = false;
    float                    step                   = 0.0F;
    float                    stepSameSpeed          = 0.0F;

    bool                     endOfShow              = false;
    bool                     showingEnd             = false;

    int                      delay                  = 0;
    bool                     disableFadeInOut       = false;
    bool                     disableCrossFade       = false;
    bool                     enableSameSpeed        = false;
    unsigned                 forceFrameRate         = 0;

    PresentationContainer*   sharedData             = nullptr;

    PresentationAudioWidget* playbackWidget         = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
