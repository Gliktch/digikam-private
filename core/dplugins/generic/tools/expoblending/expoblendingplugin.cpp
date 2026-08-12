/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "expoblendingmanager.h"

namespace DigikamGenericExpoBlendingPlugin
{

ExpoBlendingPlugin::ExpoBlendingPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void ExpoBlendingPlugin::cleanUp()
{
    if (ExpoBlendingManager::isCreated())
    {
        delete ExpoBlendingManager::internalPtr;
    }
}

QString ExpoBlendingPlugin::name() const
{
    return i18n("Exposure Blending");
}

QString ExpoBlendingPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ExpoBlendingPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("expoblending"));
}

QString ExpoBlendingPlugin::description() const
{
    return i18n("A tool to blend bracketed images");
}

QString ExpoBlendingPlugin::details() const
{
    return i18n("<p>This tool allows users to blend bracketed images together to create pseudo HDR photo.</p>"
                "<p>To create high definition range image, you need to use images from same subject "
                "taken with a tripod and exposed with different exposure settings.</p>"
                "<p>To create image with better results, you can use RAW images instead JPEG, where "
                "colors depth is higher and are well adapted for merging pixels by pixels.</p>");
}

QString ExpoBlendingPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString ExpoBlendingPlugin::handbookChapter() const
{
    return QLatin1String("expo_blending");
}

QList<DPluginAuthor> ExpoBlendingPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Johannes Wienke"),
                             QString::fromUtf8("languitar at semipol dot de"),
                             QString::fromUtf8("(C) 2010"))
            << DPluginAuthor(QString::fromUtf8("Benjamin Girault"),
                             QString::fromUtf8("benjamin dot girault at gmail dot com"),
                             QString::fromUtf8("(C) 2014"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2026"),
                             i18n("Author and Maintainer"))
            ;
}

void ExpoBlendingPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Blend Stacked Images..."));
    ac->setObjectName(QLatin1String("expoblending"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotExpoBlending()));

    addAction(ac);
}

void ExpoBlendingPlugin::slotExpoBlending()
{
    DInfoInterface* const iface = infoIface(sender());
    bool created                = ExpoBlendingManager::isCreated();

    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(iface->currentSelectedItems());
    ExpoBlendingManager::instance()->setPlugin(this);

    if (!created)
    {
        connect(ExpoBlendingManager::instance(), SIGNAL(updateHostApp(QUrl)),
                iface, SLOT(slotMetadataChangedForUrl(QUrl)));
    }

    ExpoBlendingManager::instance()->run();
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingplugin.cpp"
