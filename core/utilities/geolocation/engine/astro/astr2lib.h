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

#pragma once

// Local includes

#include "attlib.h"
#include "digikam_export.h"

namespace Marble
{

/**
 * Definitions of Astr2lib Functions
 */
class DIGIKAM_EXPORT Plan200      // Calculating the Planets in epoch J2000.0 coordinates
{
public:

    Plan200();
    Vec3 velocity();                    // return last calculated planet velocity
    void state(Vec3& rs, Vec3& vs);     // return last state vector
    Vec3 Mercury(double t);             // position of Mercury at time t
    Vec3 Venus(double t);               // position of Venus at time t
    Vec3 Mars(double t);                // position of Mars at time t
    Vec3 Jupiter(double t);             // position of Jupiter at time t
    Vec3 Saturn(double t);              // position of Saturn at time t
    Vec3 Uranus(double t);              // position of Uranus at time t
    Vec3 Neptune(double t);             // position of Neptune at time t
    Vec3 Pluto(double t);               // position of Pluto at time t

private:

    void addthe(double c1, double s1, double c2, double s2,
                double& cc, double& ss);
    void term(int i1, int i, int it, double dlc, double dls, double drc,
              double drs, double dbc, double dbs);
    void posvel();

private:

    double c3[19] = { 0.0 }, s3[19] = { 0.0 };
    double c[11]  = { 0.0 }, s[11]  = { 0.0 };

    double m1 = 0.0, m2 = 0.0, m3 = 0.0, m4 = 0.0, m5 = 0.0, m6 = 0.0, m7 = 0.0, m8 = 0.0;
    double /*d, a, uu,*/ tt = 0.0;
    double u = 0.0, v = 0.0, dl = 0.0, dr = 0.0, db = 0.0, l = 0.0, b = 0.0, r = 0.0;

    Vec3   rp, vp;   // state vector of planet
};

DIGIKAM_EXPORT void MarPhobos(double t, Vec3& rs, Vec3& vs);  // State vector of Phobos
DIGIKAM_EXPORT void MarDeimos(double t, Vec3& rs, Vec3& vs);  // State vector of Deimos
DIGIKAM_EXPORT Vec3 PosJIo(double t);       // Position of Io
DIGIKAM_EXPORT Vec3 PosEuropa(double t);    // Position of Europa
DIGIKAM_EXPORT Vec3 PosGanymede(double t);  // Position of Ganymede
DIGIKAM_EXPORT Vec3 PosCallisto(double t);  // Position of Callisto
DIGIKAM_EXPORT Vec3 PosSMimas(double t);  // Position of Mimas
DIGIKAM_EXPORT Vec3 PosSEnceladus(double t);  // Position of Enceladus
DIGIKAM_EXPORT Vec3 PosSDione(double t);  // Position of Dione
DIGIKAM_EXPORT void JupIo(double t, Vec3& rs, Vec3& vs);        // state vector of Io
DIGIKAM_EXPORT void JupEuropa(double t, Vec3& rs, Vec3& vs);    // state vector of Europa
DIGIKAM_EXPORT void JupGanymede(double t, Vec3& rs, Vec3& vs);  // state vector of Ganymede
DIGIKAM_EXPORT void JupCallisto(double t, Vec3& rs, Vec3& vs);  // state vector of Callisto
DIGIKAM_EXPORT void SatRhea(double t, Vec3& rs, Vec3& vs);    // state vector of Rhea
DIGIKAM_EXPORT void SatTitan(double t, Vec3& rs, Vec3& vs);   // state vector of Titan
DIGIKAM_EXPORT void NepTriton(double t, Vec3& rs, Vec3& vs);  // state vector of Triton
DIGIKAM_EXPORT void PluCharon(double t, Vec3& rs, Vec3& vs);  // state vector of Charon

} // namespace Marble
