/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-12
 * Description : Metadata info containers
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "metadatainfo.h"

namespace Digikam
{

bool IptcCoreContactInfo::isNull() const
{
    return (
            city.isNull()          &&
            country.isNull()       &&
            address.isNull()       &&
            postalCode.isNull()    &&
            provinceState.isNull() &&
            email.isNull()         &&
            phone.isNull()         &&
            webUrl.isNull()
           );
}

bool IptcCoreContactInfo::isEmpty() const
{
    return (
            city.isEmpty()          &&
            country.isEmpty()       &&
            address.isEmpty()       &&
            postalCode.isEmpty()    &&
            provinceState.isEmpty() &&
            email.isEmpty()         &&
            phone.isEmpty()         &&
            webUrl.isEmpty()
           );
}

bool IptcCoreContactInfo::operator==(const IptcCoreContactInfo& t) const
{
    bool b1 = (city          == t.city);
    bool b2 = (country       == t.country);
    bool b3 = (address       == t.address);
    bool b4 = (postalCode    == t.postalCode);
    bool b5 = (provinceState == t.provinceState);
    bool b6 = (email         == t.email);
    bool b7 = (phone         == t.phone);
    bool b8 = (webUrl        == t.webUrl);

    return (b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8);
}

void IptcCoreContactInfo::merge(const IptcCoreContactInfo& t, bool merge)
{
    if (!merge || (!t.city.isEmpty() && city.isEmpty()))
    {
        city = t.city;
    }

    if (!merge || (!t.country.isEmpty() && country.isEmpty()))
    {
        country = t.country;
    }

    if (!merge || (!t.address.isEmpty() && address.isEmpty()))
    {
        address = t.address;
    }

    if (!merge || (!t.postalCode.isEmpty() && postalCode.isEmpty()))
    {
        postalCode = t.postalCode;
    }

    if (!merge || (!t.provinceState.isEmpty() && provinceState.isEmpty()))
    {
        provinceState = t.provinceState;
    }

    if (!merge || (!t.email.isEmpty() && email.isEmpty()))
    {
        email = t.email;
    }

    if (!merge || (!t.phone.isEmpty() && phone.isEmpty()))
    {
        phone = t.phone;
    }

    if (!merge || (!t.webUrl.isEmpty() && webUrl.isEmpty()))
    {
        webUrl = t.webUrl;
    }
}

QDebug operator<<(QDebug dbg, const IptcCoreContactInfo& inf)
{
    dbg.nospace() << "IptcCoreContactInfo::city: "
                  << inf.city << ", ";
    dbg.nospace() << "IptcCoreContactInfo::country: "
                  << inf.country << ", ";
    dbg.nospace() << "IptcCoreContactInfo::address: "
                  << inf.address << ", ";
    dbg.nospace() << "IptcCoreContactInfo::postalCode: "
                  << inf.postalCode << ", ";
    dbg.nospace() << "IptcCoreContactInfo::provinceState: "
                  << inf.provinceState << ", ";
    dbg.nospace() << "IptcCoreContactInfo::email: "
                  << inf.email << ", ";
    dbg.nospace() << "IptcCoreContactInfo::phone: "
                  << inf.phone << ", ";
    dbg.nospace() << "IptcCoreContactInfo::webUrl: "
                  << inf.webUrl;

    return dbg.space();
}

// -------------------------------------------------------------------------

bool IptcCoreLocationInfo::isEmpty() const
{
    return (
            country.isEmpty()       &&
            countryCode.isEmpty()   &&
            provinceState.isEmpty() &&
            city.isEmpty()          &&
            location.isEmpty()
           );
}

bool IptcCoreLocationInfo::isNull() const
{
    return (
            country.isNull()       &&
            countryCode.isNull()   &&
            provinceState.isNull() &&
            city.isNull()          &&
            location.isNull()
           );
}

bool IptcCoreLocationInfo::operator==(const IptcCoreLocationInfo& t) const
{
    bool b1 = (country       == t.country);
    bool b2 = (countryCode   == t.countryCode);
    bool b3 = (provinceState == t.provinceState);
    bool b4 = (city          == t.city);
    bool b5 = (location      == t.location);

    return (b1 && b2 && b3 && b4 && b5);
}

void IptcCoreLocationInfo::merge(const IptcCoreLocationInfo& t, bool merge)
{
    if (!merge || (!t.country.isEmpty() && country.isEmpty()))
    {
        country = t.country;
    }

    if (!merge || (!t.countryCode.isEmpty() && countryCode.isEmpty()))
    {
        countryCode = t.countryCode;
    }

    if (!merge || (!t.provinceState.isEmpty() && provinceState.isEmpty()))
    {
        provinceState = t.provinceState;
    }

    if (!merge || (!t.city.isEmpty() && city.isEmpty()))
    {
        city = t.city;
    }

    if (!merge || (!t.location.isEmpty() && location.isEmpty()))
    {
        location = t.location;
    }
}

QDebug operator<<(QDebug dbg, const IptcCoreLocationInfo& inf)
{
    dbg.nospace() << "IptcCoreLocationInfo::country: "
                  << inf.country << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::countryCode: "
                  << inf.countryCode << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::provinceState: "
                  << inf.provinceState << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::city: "
                  << inf.city << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::location: "
                  << inf.location;

    return dbg.space();
}

} // namespace Digikam
