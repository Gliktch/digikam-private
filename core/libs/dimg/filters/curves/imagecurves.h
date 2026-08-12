/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSharedDataPointer>
#include <QPoint>
#include <QPolygon>
#include <QUrl>

// Local includes

#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class CurvesContainer;

class DIGIKAM_EXPORT ImageCurves
{

public:

    /**
     * The max number of points contained in a curve.
     */
    static const int NUMBER_OF_POINTS       = 17;

    /**
     * Number of channels in a curve.
     */
    static const int NUM_CHANNELS     = 5;

    /**
     * Curve points have to multiplied with this value for 16 bit images.
     */
    static const int MULTIPLIER_16BIT = 255;

    enum CurveType
    {
        CURVE_SMOOTH = 0,            ///< Smooth curve type
        CURVE_FREE                   ///< Freehand curve type.
    };

    typedef double CRMatrix[4][4];

public:

    explicit ImageCurves(bool sixteenBit);
    explicit ImageCurves(const CurvesContainer& container);
    ImageCurves(const ImageCurves& other);
    ~ImageCurves();

    ImageCurves& operator=(const ImageCurves& other);

    /**
     * Fills this curves with the data supplied by another curves object. This
     * ensures that 8 and 16 bit curves are properly converted.
     *
     * @param otherCurves other curves object to adapt config from
     */
    void fillFromOtherCurves(const ImageCurves* const otherCurves);


    /// Methods to manipulate the curves data.

    void   curvesReset();
    void   curvesChannelReset(int channel);
    void   curvesCalculateCurve(int channel);
    void   curvesCalculateAllCurves();
    float  curvesLutFunc(int n_channels, int channel, float value);
    void   curvesLutSetup(int nchannels);
    void   curvesLutProcess(uchar* const srcPR, uchar* const destPR, int w, int h);

    /// Methods to set manually the curves values.

    void   setCurveValue(int channel, int bin, int val);
    void   setCurvePointX(int channel, int point, int x);
    void   setCurvePointY(int channel, int point, int y);
    void   setCurveType(int channel, CurveType type);
    void   setCurveType(CurveType type);

    void   setCurvePoint(int channel, int point, const QPoint& val);
    void   setCurvePoints(int channel, const QPolygon& vals);
    void   setCurveValues(int channel, const QPolygon& vals);

    void   unsetCurvePoint(int channel, int point);

    /// Curves properties.

    bool   isDirty()                                     const;
    bool   isSixteenBits()                               const;

    int       getCurveValue(int channel, int bin)        const;
    int       getCurvePointX(int channel, int point)     const;
    int       getCurvePointY(int channel, int point)     const;
    CurveType getCurveType(int channel)                  const;

    bool     isCurvePointEnabled(int channel, int point) const;
    QPoint   getCurvePoint(int channel, int point)       const;
    QPolygon getCurvePoints(int channel)                 const;
    QPolygon getCurveValues(int channel)                 const;

    /// @note bits depth must match
    void   setContainer(const CurvesContainer& container);

    /**
     * Returns a container with the settings for all channels of this Curves object
     */
    CurvesContainer getContainer()                       const;

    /**
     * Returns a container containing the values of this Curves
     * object for the given channel, and linear values for all
     * other channels.
     */
    CurvesContainer getContainer(int channel)            const;

    /**
     * Returns true if the curve is linear for the given channel, or all channels.
     */
    bool isLinear(int channel)                           const;
    bool isLinear()                                      const;

    /**
     * Writes the given channel to a raw binary representation.
     * Note that 16bit free curves take a lot of memory (~85kB)
     * while all other forms take less than 400 bytes.
     */
    QByteArray channelToBinary(int channel)              const;

    /**
     * Set the channel from the given raw binary representation.
     * The data is checked for validity, only on valid data true is returned.
     * Note that the bytes depth (isSixteenBits()) of the encoded
     * representation must match the depth of this curves object.
     */
    bool setChannelFromBinary(int channel, const QByteArray& array);

    /// Methods to save/load the curves values to/from a Gimp curves text file.

    bool saveCurvesToGimpCurvesFile(const QUrl& fileUrl) const;
    bool loadCurvesFromGimpCurvesFile(const QUrl& fileUrl);

    static QPoint getDisabledValue();

private:

    void curvesPlotCurve(int channel, int p1, int p2, int p3, int p4);
    void curvesCRCompose(const CRMatrix a, const CRMatrix b, CRMatrix ab);
    void freeLutData();

private:

    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace Digikam
