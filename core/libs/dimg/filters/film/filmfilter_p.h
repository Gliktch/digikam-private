/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter filter
 *
 * SPDX-FileCopyrightText: 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "filmfilter.h"

namespace Digikam
{

class DColor;

class Q_DECL_HIDDEN FilmProfile
{
public:

    explicit FilmProfile(double rdm = 0.0, double gdm = 0.0, double bdm = 0.0)
        : redDmax  (rdm),
          greenDmax(gdm),
          blueDmax (bdm)
    {
    }

    double dmax(int channel) const
    {
        switch (channel)
        {
            case RedChannel:
            {
                return redDmax;
            }

            case GreenChannel:
            {
                return greenDmax;
            }

            case BlueChannel:
            {
                return blueDmax;
            }

            default:
            {
                return 0.0;
            }
        }
    }

    FilmProfile& setBalance(double rB, double gB, double bB)
    {
        rBalance = rB;
        gBalance = gB;
        bBalance = bB;

        return *this;
    }

    double balance(int channel) const
    {
        switch (channel)
        {
            case RedChannel:
            {
                return rBalance;
            }

            case GreenChannel:
            {
                return gBalance;
            }

            case BlueChannel:
            {
                return bBalance;
            }

            default:
            {
                return 1.0;
            }
        }
    }

    FilmProfile& setWp(double rWp, double gWp, double bWp)
    {
        wpRed   = rWp;
        wpGreen = gWp;
        wpBlue  = bWp;

        return *this;
    }

    double wp(int channel) const
    {
        switch (channel)
        {
            case RedChannel:
            {
                return wpRed;
            }

            case GreenChannel:
            {
                return wpGreen;
            }

            case BlueChannel:
            {
                return wpBlue;
            }

            default:
            {
                return 1.0;
            }
        }
    }

private:

    double redDmax      = 1.0;
    double greenDmax    = 1.0;
    double blueDmax     = 1.0;

    double rBalance     = 1.0;
    double gBalance     = 1.0;
    double bBalance     = 1.0;

    double wpRed        = 1.0;
    double wpGreen      = 1.0;
    double wpBlue       = 1.0;
};

// --------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN FilmContainer::Private
{
public:

    Private()  = default;
    ~Private() = default;

    double        gamma         = 1.0;
    double        exposure      = 1.0;
    bool          sixteenBit    = false;
    FilmProfile   profile       = FilmProfile(1.0, 1.0, 1.0);
    CNFilmProfile cnType        = CNNeutral;
    DColor        whitePoint    = DColor(QColor("white"), false);
    bool          applyBalance  = true;
};

// ------------------------------------------------------------------

class Q_DECL_HIDDEN FilmFilter::Private
{
public:

    Private()  = default;
    ~Private() = default;

    FilmContainer film;
};

} // namespace Digikam
