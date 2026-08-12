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

class DIGIKAM_EXPORT PlanetarySats     // Calculate spacecraft around other planets
{
public:

    PlanetarySats();
    ~PlanetarySats();

public:

    void setStepWidth(double s);                                                // set the step width (seconds)
    void setDeltaTAI_UTC(double d);                                             // set IERS Parameter TAI - UTC
    void setAutoTAI_UTC();                                                      // IERS Parameter TAI - UTC to auto
    void getTime();                                                             // Get System Time and Date
    void setDeltaRT(double drt);
    void setMJD(int year, int month, int day, int hour, int min, double sec);   // set time
    static void getDatefromMJD(double mjd, int& year, int& month, int& day, int& hour, int& min, double& sec);
    void setSatFile(const char* const fname);
    void setPlanet(const char* const pname);
    int selectSat(const char* const sname);
    void getSatName(char* sname) const;
    void setStateVector(double mjd, double x, double y, double z, double vx, double vy, double vz);
    int getStateVector(int nsat);
    void stateToKepler();
    void getKeplerElements(double& perc, double& apoc, double& inc, double& ecc, double& ra, double& tano, double& m0, double& a, double& n0);
    void getPlanetographic(double& lng, double& lat, double& height) const;
    void getFixedFrame(double& x, double& y, double& z, double& vx, double& vy, double& vz);
    void currentPos();
    void nextStep();
    double getLastMJD() const;

private:

    void plsatinit();                           // initialize PlanetarySats
    static double atan23(double y, double x);   // atan without singularity for x,y=0
    void getMercury();
    void getVenus();
    void getMoon();
    void getMars();
    void getSatPos(double t);
    static Mat3 getSelenographic(double jd);

private:

    // data fields

    char pls_satelmfl[205]  = { 0 };    // name of file for satellite state vectors
    char pls_satname[40]    = { 0 };    // name of satellite
    char pls_plntname[40]   = { 0 };    // name of planet

    bool pls_moonflg        = false;    // true if Moon, false if other body

    int pls_day             = 0;        // date
    int pls_month           = 0;
    int pls_year            = 0;
    int pls_hour            = 0;
    int pls_minute          = 0;
    int pls_second          = 0;
    double pls_time         = 0.0;      // current time in MJD (UTC)
    double pls_del_tdut     = 0.0;      // TDT - UT in sec
    int pls_del_auto        = 0;        // 1 = automatic del_tdut,  0 = manual
    double pls_step         = 0.0;      // stepwidth in sec
    double pls_delta_rt     = 0.0;      // delta time to R/T in hours

    double pls_tepoch       = 0.0;      // MJD epoch of state vector (TT)
    Vec3 pls_rep;                       // state vector km and km/s
    Vec3 pls_vep;

    double pls_ra           = 0.0;      // Right Ascension
    double pls_per          = 0.0;      // argument of perige center
    double pls_m0           = 0.0;      // mean anomaly
    double pls_ecc          = 0.0;      // eccentricity
    double pls_inc          = 0.0;      // inclination
    double pls_a            = 0.0;      // semi-major axis (km)
    double pls_n0           = 0.0;      // mean motion  (rev/day)

    double pls_GM           = 0.0;      // graviatational constant (m^3/s^2)
    double pls_J2           = 0.0;      // J2 gravitational term
    double pls_R0           = 0.0;      // equatorial radius (km)
    double pls_flat         = 0.0;      // flattening factor
    double pls_axl0         = 0.0;      // l-direction of rotation axis
    double pls_axl1         = 0.0;      // delta of axl0
    double pls_axb0         = 0.0;      // b-direction of rotation axis
    double pls_axb1         = 0.0;      // delta of axb0
    double pls_W            = 0.0;      // location of prime meridian
    double pls_Wd           = 0.0;      // daily variation of W.

    Vec3 pls_r;                         // current state vector  m and m/s
    Vec3 pls_v;
    double pls_lat          = 0.0;      // planetary latitude (decimal degrees)
    double pls_lng          = 0.0;      // planetary longitude (decimal degrees)
    double pls_height       = 0.0;      // height above reference ellipsoid (km)
};

} // namespace Marble
