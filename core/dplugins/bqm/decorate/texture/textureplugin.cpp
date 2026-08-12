/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-13
 * Description : a BQM plugin to add texture
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textureplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "texture.h"

namespace DigikamBqmTexturePlugin
{

TexturePlugin::TexturePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString TexturePlugin::name() const
{
    return i18nc("@title", "Add Texture");
}

QString TexturePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TexturePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("texture"));
}

QString TexturePlugin::description() const
{
    return i18nc("@info", "A tool to apply a texture over images");
}

QString TexturePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can apply a texture over images.</para>");
}

QString TexturePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString TexturePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString TexturePlugin::handbookReference() const
{
    return QLatin1String("bqm-decoratetools");
}

QList<DPluginAuthor> TexturePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void TexturePlugin::setup(QObject* const parent)
{
    Texture* const tool = new Texture(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmTexturePlugin

#include "moc_textureplugin.cpp"
