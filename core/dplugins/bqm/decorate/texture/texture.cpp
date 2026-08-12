/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-13
 * Description : batch tool to add texture.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "texture.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "texturefilter.h"

namespace DigikamBqmTexturePlugin
{

Texture::Texture(QObject* const parent)
    : BatchTool(QLatin1String("Texture"), DecorateTool, parent)
{
}

BatchTool* Texture::clone(QObject* const parent) const
{
    return new Texture(parent);
}

void Texture::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new TextureSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Texture::defaultSettings()
{
    BatchToolSettings prm;
    TextureContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("blendGain"),   defaultPrm.blendGain);
    prm.insert(QLatin1String("textureType"), defaultPrm.textureType);

    return prm;
}

void Texture::slotAssignSettings2Widget()
{
    TextureContainer prm;

    prm.blendGain   = settings().value(QLatin1String("blendGain")).toInt();
    prm.textureType = settings().value(QLatin1String("textureType")).toInt();

    m_settingsView->setSettings(prm);
}

void Texture::slotSettingsChanged()
{
    BatchToolSettings prm;
    TextureContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("blendGain"),   currentPrm.blendGain);
    prm.insert(QLatin1String("textureType"), currentPrm.textureType);

    BatchTool::slotSettingsChanged(prm);
}

bool Texture::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    TextureContainer prm;
    prm.blendGain   = settings().value(QLatin1String("blendGain")).toInt();
    prm.textureType = settings().value(QLatin1String("textureType")).toInt();

    TextureFilter bd(&image(), nullptr, prm);
    applyFilter(&bd);

    return (savefromDImg());
}

} // namespace DigikamBqmTexturePlugin

#include "moc_texture.cpp"
