/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "MarbleGlobal.h"

// Local includes

#include "MarbleLocale.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleGlobalPrivate
{
public:

    MarbleGlobalPrivate();
    virtual ~MarbleGlobalPrivate();

    MarbleLocale           m_locale;
    MarbleGlobal::Profiles m_profiles;
};

MarbleGlobalPrivate::MarbleGlobalPrivate()
    : m_profiles(MarbleGlobal::Default)
{

#ifdef Q_OS_ANDROID

    m_profiles |= MarbleGlobal::SmallScreen;
    m_profiles |= MarbleGlobal::HighResolution;

#endif

}

MarbleGlobalPrivate::~MarbleGlobalPrivate()
{
}

// ---

MarbleGlobal::MarbleGlobal()
    : d(new MarbleGlobalPrivate)
{
}

MarbleGlobal::~MarbleGlobal()
{
    delete d;
}

MarbleGlobal* MarbleGlobal::getInstance()
{
    static MarbleGlobal instance;

    return &instance;
}

MarbleLocale* MarbleGlobal::locale() const
{
    return &d->m_locale;
}

MarbleGlobal::Profiles MarbleGlobal::profiles() const
{
    return d->m_profiles;
}

void MarbleGlobal::setProfiles(MarbleGlobal::Profiles profiles)
{
    d->m_profiles = profiles;
}

} // namespace Marble
