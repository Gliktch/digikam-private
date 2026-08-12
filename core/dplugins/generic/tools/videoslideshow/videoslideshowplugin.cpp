/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to create video slideshow.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "videoslideshowplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"

namespace DigikamGenericVideoSlideShowPlugin
{

VideoSlideShowPlugin::VideoSlideShowPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void VideoSlideShowPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString VideoSlideShowPlugin::name() const
{
    return i18n("Video Slideshow");
}

QString VideoSlideShowPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon VideoSlideShowPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("media-record"));
}

QString VideoSlideShowPlugin::description() const
{
    return i18n("A tool to create video slideshow from images");
}

QString VideoSlideShowPlugin::details() const
{
    return i18n("<p>This tool allows users to back-process image as frame to create video slide-show.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different visual effects can be applied to images to make transitions while to render target video.</p>");
}

QString VideoSlideShowPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString VideoSlideShowPlugin::handbookChapter() const
{
    return QLatin1String("video_slideshow");
}

QList<DPluginAuthor> VideoSlideShowPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("A Janardhan Reddy"),
                             QString::fromUtf8("annapareddyjanardhanreddy at gmail dot com"),
                             QString::fromUtf8("(C) 2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2025"),
                             i18n("Author and Maintainer"))
            ;
}

void VideoSlideShowPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create video slideshow..."));
    ac->setObjectName(QLatin1String("videoslideshow"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotVideoSlideShow()));

    addAction(ac);
}

void VideoSlideShowPlugin::slotVideoSlideShow()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new VidSlideWizard(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_videoslideshowplugin.cpp"
