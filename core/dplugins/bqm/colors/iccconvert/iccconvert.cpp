/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Color profile conversion tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccconvert.h"

// Qt includes

#include <QLabel>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "icctransform.h"
#include "icctransformfilter.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"

namespace DigikamBqmIccConvertPlugin
{

IccConvert::IccConvert(QObject* const parent)
    : BatchTool(QLatin1String("IccConvert"), ColorTool, parent)
{
}

BatchTool* IccConvert::clone(QObject* const parent) const
{
    return new IccConvert(parent);
}

void IccConvert::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    m_settingsView      = new IccProfilesSettings(vbox);
    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);
    m_settingsWidget = vbox;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings IccConvert::defaultSettings()
{
    BatchToolSettings prm;
    IccProfile defaultProf = m_settingsView->defaultProfile();

    prm.insert(QLatin1String("ProfilePath"), defaultProf.filePath());

    return prm;
}

void IccConvert::slotAssignSettings2Widget()
{
    QString profPath = settings().value(QLatin1String("ProfilePath")).toString();
    m_settingsView->setCurrentProfile(IccProfile(profPath));
}

void IccConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    IccProfile currentProf = m_settingsView->currentProfile();

    prm.insert(QLatin1String("ProfilePath"), currentProf.filePath());

    BatchTool::slotSettingsChanged(prm);
}

bool IccConvert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    QString              profPath = settings().value(QLatin1String("ProfilePath")).toString();
    IccProfile           in       = image().getIccProfile();
    IccProfile           out(profPath);
    ICCSettingsContainer settings = IccSettings::instance()->settings();
    IccTransform         transform;

    transform.setIntent(settings.renderingIntent);
    transform.setUseBlackPointCompensation(settings.useBPC);
    transform.setInputProfile(in);
    transform.setOutputProfile(out);

    IccTransformFilter icc(&image(), nullptr, transform);
    applyFilter(&icc);

    image().setIccProfile(icc.getTargetImage().getIccProfile());

    QScopedPointer<DMetadata> meta(new DMetadata(image().getMetadata()));
    meta->removeExifColorSpace();
    image().setMetadata(meta->data());

    return (savefromDImg());
}

} // namespace DigikamBqmIccConvertPlugin

#include "moc_iccconvert.cpp"
