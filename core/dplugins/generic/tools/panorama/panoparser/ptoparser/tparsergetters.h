/*============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Description : Hugin parser API
 *
 * SPDX-FileCopyrightText: 2007 by Daniel M German <dmgerman at uvic doooot ca>
 * SPDX-FileCopyrightText: 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "tparser.h"

/**
 * NOTE: any function name ending with "Ref" returns -1 if there are no such reference
 */

int     panoScriptGetImagesCount(const pt_script* const script);
int     panoScriptGetImagePrevCommentsCount(const pt_script* const script, int i);
char*   panoScriptGetImageComment(const pt_script* const script, int i, int c);

/**
 * 0: Rectilinear, 1: Panoramic, 2: Circular fisheye, 3: FF fisheye, 4: equirectangular
 */
int     panoScriptGetImageProjection(const pt_script* const script, int i);
int     panoScriptGetImageWidth(const pt_script* const script, int i);
int     panoScriptGetImageHeight(const pt_script* const script, int i);
double  panoScriptGetImageHFOV(const pt_script* const script, int i);
int     panoScriptGetImageHFOVRef(const pt_script* const script, int i);
double  panoScriptGetImageYaw(const pt_script* const script, int i);
double  panoScriptGetImagePitch(const pt_script* const script, int i);
double  panoScriptGetImageRoll(const pt_script* const script, int i);
double  panoScriptGetImageCoefA(const pt_script* const script, int i);
int     panoScriptGetImageCoefARef(const pt_script* const script, int i);
double  panoScriptGetImageCoefB(const pt_script* const script, int i);
int     panoScriptGetImageCoefBRef(const pt_script* const script, int i);
double  panoScriptGetImageCoefC(const pt_script* const script, int i);
int     panoScriptGetImageCoefCRef(const pt_script* const script, int i);
double  panoScriptGetImageCoefD(const pt_script* const script, int i);
int     panoScriptGetImageCoefDRef(const pt_script* const script, int i);
double  panoScriptGetImageCoefE(const pt_script* const script, int i);
int     panoScriptGetImageCoefERef(const pt_script* const script, int i);
double  panoScriptGetImageSheerX(const pt_script* const script, int i);
int     panoScriptGetImageSheerXRef(const pt_script* const script, int i);
double  panoScriptGetImageSheerY(const pt_script* const script, int i);
int     panoScriptGetImageSheerYRef(const pt_script* const script, int i);
double  panoScriptGetImageExposure(const pt_script* const script, int i);
int     panoScriptGetImageExposureRef(const pt_script* const script, int i);
double  panoScriptGetImageWBRed(const pt_script* const script, int i);
int     panoScriptGetImageWBRedRef(const pt_script* const script, int i);
double  panoScriptGetImageWBBlue(const pt_script* const script, int i);
int     panoScriptGetImageWBBlueRef(const pt_script* const script, int i);
double  panoScriptGetImagePhotometricCoeffA(const pt_script* const script, int i);
int     panoScriptGetImagePhotometricCoeffARef(const pt_script* const script, int i);
double  panoScriptGetImagePhotometricCoeffB(const pt_script* const script, int i);
int     panoScriptGetImagePhotometricCoeffBRef(const pt_script* const script, int i);
double  panoScriptGetImagePhotometricCoeffC(const pt_script* const script, int i);
int     panoScriptGetImagePhotometricCoeffCRef(const pt_script* const script, int i);
double  panoScriptGetImagePhotometricCoeffD(const pt_script* const script, int i);
int     panoScriptGetImagePhotometricCoeffDRef(const pt_script* const script, int i);
double  panoScriptGetImagePhotometricCoeffE(const pt_script* const script, int i);
int     panoScriptGetImagePhotometricCoeffERef(const pt_script* const script, int i);

/**
 * Bit0: radial, Bit1: flatfield, Bit2: proportional
 */
int     panoScriptGetImageVignettingMode(const pt_script* const script, int i);
int     panoScriptGetImageVignettingModeRef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffA(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffARef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffB(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffBRef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffC(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffCRef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffD(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffDRef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffX(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffXRef(const pt_script* const script, int i);
double  panoScriptGetImageVignettingCoeffY(const pt_script* const script, int i);
int     panoScriptGetImageVignettingCoeffYRef(const pt_script* const script, int i);
char*   panoScriptGetImageVignettingFlatField(const pt_script* const script, int i);
double  panoScriptGetImageCameraTranslationX(const pt_script* const script, int i);
double  panoScriptGetImageCameraTranslationY(const pt_script* const script, int i);
double  panoScriptGetImageCameraTranslationZ(const pt_script* const script, int i);
double  panoScriptGetImageProjectionPlaneYaw(const pt_script* const script, int i);
double  panoScriptGetImageProjectionPlanePitch(const pt_script* const script, int i);
char*   panoScriptGetImageName(const pt_script* const script, int i);
int     panoScriptGetImageCropLeft(const pt_script* const script, int i);
int     panoScriptGetImageCropRight(const pt_script* const script, int i);
int     panoScriptGetImageCropTop(const pt_script* const script, int i);
int     panoScriptGetImageCropBottom(const pt_script* const script, int i);
int     panoScriptGetImageStack(const pt_script* const script, int i);
int     panoScriptGetImageStackRef(const pt_script* const script, int i);

int     panoScriptGetPanoPrevCommentsCount(const pt_script* const script);
char*   panoScriptGetPanoComment(const pt_script* const script, int c);
int     panoScriptGetPanoWidth(const pt_script* const script);
int     panoScriptGetPanoHeight(const pt_script* const script);
int     panoScriptGetPanoCropLeft(const pt_script* const script);
int     panoScriptGetPanoCropRight(const pt_script* const script);
int     panoScriptGetPanoCropTop(const pt_script* const script);
int     panoScriptGetPanoCropBottom(const pt_script* const script);
int     panoScriptGetPanoProjection(const pt_script* const script);
double  panoScriptGetPanoProjectionParmsCount(const pt_script* const script);
double  panoScriptGetPanoProjectionParm(const pt_script* const script, int index);
double  panoScriptGetPanoHFOV(const pt_script* const script);
int     panoScriptGetPanoOutputFormat(const pt_script* const script);           /* 0: PNG, 1: TIFF, 2: TIFF_m, 3: TIFF_multilayer, 4: JPEG */
int     panoScriptGetPanoOutputCompression(const pt_script* const script);      /* 0: PANO_NONE, 1: LZW, 2: DEFLATE */
int     panoScriptGetPanoOutputCropped(const pt_script* const script);
int     panoScriptGetPanoOutputSaveCoordinates(const pt_script* const script);
int     panoScriptGetPanoOutputQuality(const pt_script* const script);
int     panoScriptGetPanoIsHDR(const pt_script* const script);
int     panoScriptGetPanoBitDepth(const pt_script* const script);               /* 0: 8bit, 1: 16bits,2: float */
double  panoScriptGetPanoExposure(const pt_script* const script);
int     panoScriptGetPanoImageReference(const pt_script* const script);

int     panoScriptGetOptimizePrevCommentsCount(const pt_script* const script);
char*   panoScriptGetOptimizeComment(const pt_script* const script, int c);
double  panoScriptGetOptimizeGamma(const pt_script* const script);

/**
 * 0: poly3, 1: spline16, 2: spline36, 3: sinc256, 4: spline64, 5: bilinear, 6: nearest neighbor, 7: sinc1024
 */
int     panoScriptGetOptimizeInterpolator(const pt_script* const script);
int     panoScriptGetOptimizeSpeedUp(const pt_script* const script);            /* 0: no speedup, 1: medium speedup, 2: maximum speedup */
double  panoScriptGetOptimizeHuberSigma(const pt_script* const script);
double  panoScriptGetOptimizePhotometricHuberSigma(const pt_script* const script);

int     panoScriptGetVarsToOptimizeCount(const pt_script* const script);
int     panoScriptGetVarsToOptimizePrevCommentCount(const pt_script* const script, int v);
char*   panoScriptGetVarsToOptimizeComment(const pt_script* const script, int v, int c);
int     panoScriptGetVarsToOptimizeImageId(const pt_script* const script, int v);

/**
 * 0-4: Lens A-E, 5: hfov, 6-8: yaw / pitch / roll, 9: exposure, 10-11: WB (red / blue)
 * 12-15: Vignetting A-D, 16-17: Vignetting X-Y, 18-22: photometric A-E, 23: unknown
 */
int     panoScriptGetVarsToOptimizeName(const pt_script* const script, int v);

int     panoScriptGetCtrlPointCount(const pt_script* const script);
int     panoScriptGetCtrlPointPrevCommentCount(const pt_script* const script, int cp);
char*   panoScriptGetCtrlPointComment(const pt_script* const script, int cp, int c);
int     panoScriptGetCtrlPointImage1(const pt_script* const script, int cp);
int     panoScriptGetCtrlPointImage2(const pt_script* const script, int cp);
double  panoScriptGetCtrlPointX1(const pt_script* const script, int cp);
double  panoScriptGetCtrlPointX2(const pt_script* const script, int cp);
double  panoScriptGetCtrlPointY1(const pt_script* const script, int cp);
double  panoScriptGetCtrlPointY2(const pt_script* const script, int cp);
int     panoScriptGetCtrlPointType(const pt_script* const script, int cp);

int     panoScriptGetMaskCount(const pt_script* const script);
int     panoScriptGetMaskPrevCommentCount(const pt_script* const script, int m);
char*   panoScriptGetMaskComment(const pt_script* const script, int m, int c);
int     panoScriptGetMaskImage(const pt_script* const script, int m);
int     panoScriptGetMaskType(const pt_script* const script, int m);            /* bit0: positive, bit1: stackaware, bit2(only): negativelens */
int     panoScriptGetMaskPointCount(const pt_script* const script, int m);
int     panoScriptGetMaskPointX(const pt_script* const script, int m, int p);
int     panoScriptGetMaskPointY(const pt_script* const script, int m, int p);

int     panoScriptGetEndingCommentCount(const pt_script* const script);
char*   panoScriptGetEndingComment(const pt_script* const script, int c);
