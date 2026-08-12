/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

void ApplicationSettings::setSyncBalooToDigikam(bool val)
{
    d->syncToDigikam = val;

    Q_EMIT balooSettingsChanged();
}

bool ApplicationSettings::getSyncBalooToDigikam() const
{
    return d->syncToDigikam;
}

void ApplicationSettings::setSyncDigikamToBaloo(bool val)
{
    d->syncToBaloo = val;

    Q_EMIT balooSettingsChanged();
}

bool ApplicationSettings::getSyncDigikamToBaloo() const
{
    return d->syncToBaloo;
}

DbEngineParameters ApplicationSettings::getDbEngineParameters() const
{
    return d->databaseParams;
}

void ApplicationSettings::setDbEngineParameters(const DbEngineParameters& params)
{
    d->databaseParams = params;
}

} // namespace Digikam
