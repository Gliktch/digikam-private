/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam Survey tool - Extra tools
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "surveywindow_p.h"

namespace Digikam
{

void SurveyWindow::slotEditItem()
{
    if (!d->stack->thumbBar()->currentInfo().isNull())
    {
        slotEditItem(d->stack->thumbBar()->currentInfo());
    }
}

void SurveyWindow::slotEditItem(const ItemInfo& info)
{
    ImageWindow* const im = ImageWindow::imageWindow();
    ItemInfoList list     = d->stack->thumbBar()->allItemInfos();

    im->loadItemInfos(list, info, i18n("Light Table"));

    if (im->isHidden())
    {
        im->show();
    }
    else
    {
        im->raise();
    }

    im->setFocus();
}

void SurveyWindow::slotSlideShowManualFromCurrent()
{
    QList<DPluginAction*> actions = DPluginLoader::instance()->
                                    pluginActions(QLatin1String("org.kde.digikam.plugin.generic.SlideShow"), this);

    if (actions.isEmpty())
    {
        return;
    }

    // set current image to SlideShow Plugin

    actions[0]->setData(d->stack->thumbBar()->currentInfo().fileUrl());
    actions[0]->trigger();
}

void SurveyWindow::slotSlideShowLastItemUrl()
{
    QList<DPluginAction*> actions = DPluginLoader::instance()->
                                    pluginActions(QLatin1String("org.kde.digikam.plugin.generic.SlideShow"), this);

    if (actions.isEmpty())
    {
        return;
    }

    // get last image to SlideShow Plugin

    QUrl url = actions[0]->data().toUrl();
    d->stack->thumbBar()->setCurrentUrl(url);
}

} // namespace Digikam
