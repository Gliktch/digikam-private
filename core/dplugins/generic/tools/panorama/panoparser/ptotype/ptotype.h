/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-04
 * Description : a tool to create panorama by fusion of several images.
 *               This type is based on pto file format described here:
 *               hugin.sourceforge.net/docs/nona/nona.txt, and
 *               on pto files produced by Hugin's tools.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPoint>
#include <QVector>
#include <QSize>
#include <QString>
#include <QRect>
#include <QStringList>
#include <QPair>
#include <QTextStream>

namespace Digikam
{

struct PTOType
{
    struct Project
    {
        struct FileFormat
        {
            typedef enum
            {
                PNG,
                TIFF,
                TIFF_m,
                TIFF_multilayer,
                JPEG
            } FileType;

            typedef enum
            {
                PANO_NONE,
                LZW,
                DEFLATE
            } CompressionMethod;

            FileFormat() = default;

            FileType          fileType          = JPEG;
            unsigned char     quality           = 90;       ///< for JPEG
            CompressionMethod compressionMethod = LZW;      ///< for TIFF
            bool              cropped           = false;    ///< for TIFF
            bool              savePositions     = false;    ///< for TIFF
        };

        typedef enum
        {
            RECTILINEAR      = 0,
            CYLINDRICAL      = 1,
            EQUIRECTANGULAR  = 2,
            FULLFRAMEFISHEYE = 3
        } ProjectionType;

        typedef enum
        {
            UINT8,
            UINT16,
            FLOAT
        } BitDepth;

        Project() = default;

        QStringList    previousComments;
        QSize          size                     = QSize(0, 0);
        QRect          crop                     = QRect(0, 0, 0, 0);
        ProjectionType projection               = RECTILINEAR;
        double         fieldOfView              = 0.0;
        FileFormat     fileFormat;
        double         exposure                 = 0.0;
        bool           hdr                      = false;
        BitDepth       bitDepth                 = UINT8;
        int            photometricReferenceId   = 0;
        QStringList    unmatchedParameters;
    };

    // --------------------------------------------------------------------------------------

    struct Stitcher
    {
        typedef enum
        {
            POLY3           = 0,
            SPLINE16        = 1,
            SPLINE36        = 2,
            SINC256         = 3,
            SPLINE64        = 4,
            BILINEAR        = 5,
            NEARESTNEIGHBOR = 6,
            SINC1024        = 7
        } Interpolator;

        typedef enum
        {
            SLOW,
            MEDIUM = 1,
            FAST   = 2
        } SpeedUp;

        Stitcher() = default;

        QStringList  previousComments;
        double       gamma                  = 1.0;
        Interpolator interpolator           = POLY3;
        SpeedUp      speedUp                = FAST;
        double       huberSigma             = 0.0;
        double       photometricHuberSigma  = 0.0;
        QStringList  unmatchedParameters;
    };

    // --------------------------------------------------------------------------------------

    struct Mask
    {
        typedef enum
        {
            NEGATIVE      = 0,
            POSITIVE      = 1,
            NEGATIVESTACK = 2,
            POSITIVESTACK = 3,
            NEGATIVELENS  = 4
        } MaskType;

        Mask() = default;

        QStringList   previousComments;
        MaskType      type = NEGATIVE;
        QList<QPoint> hull;
    };

    // --------------------------------------------------------------------------------------

    struct Optimization
    {
        typedef enum
        {
            LENSA,
            LENSB,
            LENSC,
            LENSD,
            LENSE,
            LENSHFOV,
            LENSYAW,
            LENSPITCH,
            LENSROLL,
            EXPOSURE,
            WBR,
            WBB,
            VA,
            VB,
            VC,
            VD,
            VX,
            VY,
            RA,
            RB,
            RC,
            RD,
            RE,
            UNKNOWN
        } Parameter;

        Optimization() = default;

        QStringList previousComments;
        Parameter   parameter = UNKNOWN;
    };

    // --------------------------------------------------------------------------------------

    struct Image
    {
        template<typename T>
        struct LensParameter
        {
            LensParameter()
              : value(T())
            {
            }

            explicit LensParameter(const T& v)
              : value(v)
            {
            }

            T   value;
            int referenceId = -1;

            friend QTextStream& operator<<(QTextStream& qts, const LensParameter<T>& p)
            {
               if (p.referenceId == -1)
               {
                   qts << p.value;
               }
               else
               {
                   qts << "=" << p.referenceId;
               }

               return qts;
            }
        };

        typedef enum
        {
            RECTILINEAR      = 0,
            PANORAMIC        = 1,
            CIRCULARFISHEYE  = 2,
            FULLFRAMEFISHEYE = 3,
            EQUIRECTANGULAR  = 4
        } LensProjection;

        typedef enum
        {
            PANO_NONE              = 0,
            RADIAL                 = 1,
            FLATFIELD              = 2,
            PROPORTIONNALRADIAL    = 5,
            PROPORTIONNALFLATFIELD = 6
        } VignettingMode;

        Image()
          : fieldOfView               (0.0),
            lensBarrelCoefficientA    (0.0),
            lensBarrelCoefficientB    (0.0),
            lensBarrelCoefficientC    (0.0),
            lensCenterOffsetX         (0),
            lensCenterOffsetY         (0),
            lensShearX                (0),
            lensShearY                (0),
            exposure                  (0.0),
            whiteBalanceRed           (1.0),
            whiteBalanceBlue          (1.0),
            vignettingMode            (PANO_NONE),
            vignettingCorrectionI     (0.0),
            vignettingCorrectionJ     (0.0),
            vignettingCorrectionK     (0.0),
            vignettingCorrectionL     (0.0),
            vignettingOffsetX         (0),
            vignettingOffsetY         (0),
            photometricEMoRA          (0.0),
            photometricEMoRB          (0.0),
            photometricEMoRC          (0.0),
            photometricEMoRD          (0.0),
            photometricEMoRE          (0.0),
            stackNumber               (0)
        {
        }

        QStringList                   previousComments;
        QSize                         size                          = QSize(0, 0);
        int                           id                            = 0;
        QList<Mask>                   masks;
        QList<Optimization>           optimizationParameters;
        LensProjection                lensProjection                = RECTILINEAR;
        LensParameter<double>         fieldOfView;
        double                        yaw                           = 0.0;
        double                        pitch                         = 0.0;
        double                        roll                          = 0.0;
        LensParameter<double>         lensBarrelCoefficientA;
        LensParameter<double>         lensBarrelCoefficientB;
        LensParameter<double>         lensBarrelCoefficientC;
        LensParameter<int>            lensCenterOffsetX;
        LensParameter<int>            lensCenterOffsetY;
        LensParameter<int>            lensShearX;
        LensParameter<int>            lensShearY;
        LensParameter<double>         exposure;
        LensParameter<double>         whiteBalanceRed;
        LensParameter<double>         whiteBalanceBlue;
        LensParameter<VignettingMode> vignettingMode;
        LensParameter<double>         vignettingCorrectionI;          ///< Va
        LensParameter<double>         vignettingCorrectionJ;          ///< Vb
        LensParameter<double>         vignettingCorrectionK;          ///< Vc
        LensParameter<double>         vignettingCorrectionL;          ///< Vd
        LensParameter<int>            vignettingOffsetX;
        LensParameter<int>            vignettingOffsetY;
        QString                       vignettingFlatfieldImageName;
        LensParameter<double>         photometricEMoRA;
        LensParameter<double>         photometricEMoRB;
        LensParameter<double>         photometricEMoRC;
        LensParameter<double>         photometricEMoRD;
        LensParameter<double>         photometricEMoRE;
        double                        mosaicCameraPositionX         = 0.0;
        double                        mosaicCameraPositionY         = 0.0;
        double                        mosaicCameraPositionZ         = 0.0;
        double                        mosaicProjectionPlaneYaw      = 0.0;
        double                        mosaicProjectionPlanePitch    = 0.0;
        QRect                         crop                          = QRect(0, 0, 0, 0);
        LensParameter<int>            stackNumber;
        QString                       fileName;
        QStringList                   unmatchedParameters;
    };

    // --------------------------------------------------------------------------------------

    struct ControlPoint
    {
        QStringList previousComments;
        int         image1Id            = 0;
        int         image2Id            = 0;
        double      p1_x                = 0.0;
        double      p1_y                = 0.0;
        double      p2_x                = 0.0;
        double      p2_y                = 0.0;
        int         type                = 0;             // FIXME: change that for an enum if possible
        QStringList unmatchedParameters;
    };

    // --------------------------------------------------------------------------------------

    enum
    {
        PRE_V2014,
        V2014
    } version;

    PTOType()
      : version(PRE_V2014)
    {
    }

    explicit PTOType(const QString& version)
      : version((version.split(QLatin1Char('.'))[0].toInt() >= 2014) ? V2014
                                                                     : PRE_V2014)
    {
    }

    bool createFile(const QString& filepath);

/*
    NOTE: Work in progress
    QPair<double, int>  standardDeviation(int image1Id, int image2Id);
    QPair<double, int>  standardDeviation(int imageId);
    QPair<double, int>  standardDeviation();
*/

    Project             project;
    Stitcher            stitcher;
    QVector<Image>      images;
    QList<ControlPoint> controlPoints;
    QStringList         lastComments;
};

} // namespace Digikam
