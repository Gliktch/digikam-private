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

#include "tparsergetters.h"

// C includes

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

// krazy:excludeall=qmacros

int panoScriptGetImagesCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iInputImagesCount;
}

int panoScriptGetImagePrevCommentsCount(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->iImage_prevCommentsCount[i];
}

char* panoScriptGetImageComment(const pt_script* const script, int i, int c)
{
    assert((c >= 0) && (c < panoScriptGetImagePrevCommentsCount(script, i)));

    return script->image_prevComments[i][c];
}

int panoScriptGetImageProjection(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].projection;
}

int panoScriptGetImageWidth(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].width;
}

int panoScriptGetImageHeight(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].height;
}

double panoScriptGetImageHFOV(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].fHorFOV;
}

int panoScriptGetImageHFOVRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].fHorFOVRef;
}

double panoScriptGetImageYaw(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].yaw;
}

double panoScriptGetImagePitch(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].pitch;
}

double panoScriptGetImageRoll(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].roll;
}

double panoScriptGetImageCoefA(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[0];
}

int panoScriptGetImageCoefARef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[0];
}

double panoScriptGetImageCoefB(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[1];
}

int panoScriptGetImageCoefBRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[1];
}

double panoScriptGetImageCoefC(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[2];
}

int panoScriptGetImageCoefCRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[2];
}

double panoScriptGetImageCoefD(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[3];
}

int panoScriptGetImageCoefDRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[3];
}

double panoScriptGetImageCoefE(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[4];
}

int panoScriptGetImageCoefERef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[4];
}

double panoScriptGetImageSheerX(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[5];
}

int panoScriptGetImageSheerXRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[5];
}

double panoScriptGetImageSheerY(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoef[6];
}

int panoScriptGetImageSheerYRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].geometryCoefRef[6];
}

double panoScriptGetImageExposure(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].imageEV;
}

int panoScriptGetImageExposureRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].imageEVRef;
}

double panoScriptGetImageWBRed(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].whiteBalanceFactorRed;
}

int panoScriptGetImageWBRedRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].whiteBalanceFactorRedRef;
}

double panoScriptGetImageWBBlue(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].whiteBalanceFactorBlue;
}

int panoScriptGetImageWBBlueRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].whiteBalanceFactorBlueRef;
}

double panoScriptGetImagePhotometricCoeffA(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoef[0];
}

int panoScriptGetImagePhotometricCoeffARef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoefRef[0];
}

double panoScriptGetImagePhotometricCoeffB(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoef[1];
}

int panoScriptGetImagePhotometricCoeffBRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoefRef[1];
}

double panoScriptGetImagePhotometricCoeffC(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoef[2];
}

int panoScriptGetImagePhotometricCoeffCRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoefRef[2];
}

double panoScriptGetImagePhotometricCoeffD(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoef[3];
}

int panoScriptGetImagePhotometricCoeffDRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoefRef[3];
}

double panoScriptGetImagePhotometricCoeffE(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoef[4];
}

int panoScriptGetImagePhotometricCoeffERef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].photometricCoefRef[4];
}

int panoScriptGetImageVignettingMode(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionMode;
}

int panoScriptGetImageVignettingModeRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionModeRef;
}

double panoScriptGetImageVignettingCoeffA(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[0];
}

int panoScriptGetImageVignettingCoeffARef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[0];
}

double panoScriptGetImageVignettingCoeffB(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[1];
}

int panoScriptGetImageVignettingCoeffBRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[1];
}

double panoScriptGetImageVignettingCoeffC(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[2];
}

int panoScriptGetImageVignettingCoeffCRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[2];
}

double panoScriptGetImageVignettingCoeffD(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[3];
}

int panoScriptGetImageVignettingCoeffDRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[3];
}

double panoScriptGetImageVignettingCoeffX(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[4];
}

int panoScriptGetImageVignettingCoeffXRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[4];
}

double panoScriptGetImageVignettingCoeffY(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoef[5];
}

int panoScriptGetImageVignettingCoeffYRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingCorrectionCoefRef[5];
}

char* panoScriptGetImageVignettingFlatField(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].vignettingFlatFieldFile;
}

double panoScriptGetImageCameraTranslationX(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cameraPosition[0];
}

double panoScriptGetImageCameraTranslationY(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cameraPosition[1];
}

double panoScriptGetImageCameraTranslationZ(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cameraPosition[2];
}

double panoScriptGetImageProjectionPlaneYaw(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].projectionPlaneRotation[0];
}

double panoScriptGetImageProjectionPlanePitch(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].projectionPlaneRotation[1];
}

char* panoScriptGetImageName(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].name;
}

int panoScriptGetImageCropLeft(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cropArea[0];
}

int panoScriptGetImageCropRight(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cropArea[1];
}

int panoScriptGetImageCropTop(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cropArea[2];
}

int panoScriptGetImageCropBottom(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].cropArea[3];
}

int panoScriptGetImageStack(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));
    return script->inputImageSpec[i].stack;
}

int panoScriptGetImageStackRef(const pt_script* const script, int i)
{
    assert((script != NULL) && (i >= 0) && (i < script->iInputImagesCount));

    return script->inputImageSpec[i].stackRef;
}

int panoScriptGetPanoPrevCommentsCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iPano_prevCommentsCount;
}

char* panoScriptGetPanoComment(const pt_script* const script, int c)
{
    assert((script != NULL) && (c >= 0) && (c < panoScriptGetPanoPrevCommentsCount(script)));

    return script->pano_prevComments[c];
}

int panoScriptGetPanoWidth(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.width;
}

int panoScriptGetPanoHeight(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.height;
}

int panoScriptGetPanoCropLeft(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.cropArea[0];
}

int panoScriptGetPanoCropRight(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.cropArea[1];
}

int panoScriptGetPanoCropTop(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.cropArea[2];
}

int panoScriptGetPanoCropBottom(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.cropArea[3];
}

int panoScriptGetPanoProjection(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.projection;
}

double panoScriptGetPanoProjectionParmsCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.projectionParmsCount;
}

double panoScriptGetPanoProjectionParm(const pt_script* const script, int index)
{
    assert((script != NULL) && (index < script->pano.projectionParmsCount));

    return script->pano.projectionParms[index];
}

double panoScriptGetPanoHFOV(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.fHorFOV;
}

int panoScriptGetPanoOutputFormat(const pt_script* const script)
{
    assert(script != NULL);

    const char* str = script->pano.outputFormat;

    if (str == 0)
    {
        return 4;
    }

    switch (str[0])
    {
        case 'P':
        {
            if (strncmp("NG", str + 1, 2) == 0)
                return 0;

            break;
        }

        case 'T':
        {
            if (strncmp("IFF", str + 1, 3) == 0)
            {
                if (strncmp("_m", str + 4, 2) == 0)
                {
                    if (strncmp("ultilayer", str + 6, 9) == 0)
                    {
                        return 3;
                    }

                    return 2;
                }

                return 1;
            }

            break;
        }

        case 'J':
        {
            if (strncmp("PEG", str + 1, 3) == 0)
                return 4;

            break;
        }

        default:
        {
            break;
        }
    }

    return -1;
}

int panoScriptGetPanoOutputCompression(const pt_script* const script)
{
    assert((script != NULL) && (panoScriptGetPanoOutputFormat(script) > 0) && (panoScriptGetPanoOutputFormat(script) < 4));

    char* str = script->pano.outputFormat;

    while (str != NULL)
    {
        str = strchr(str, ' ');

        if (str != NULL)
        {
            if (str[1] == 'c' && str[2] == ':')
            {
                str += 3;

                switch (str[0])
                {
                    case 'N':
                    {
                        if (strncmp("ONE", str + 1, 3) == 0)
                        {
                            return 0;
                        }

                        break;
                    }

                    case 'L':
                    {
                        if (strncmp("ZW", str + 1, 2) == 0)
                        {
                            return 1;
                        }

                        break;
                    }

                    case 'D':
                    {
                        if (strncmp("EFLATE", str + 1, 6) == 0)
                        {
                            return 2;
                        }

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                return -1;
            }

            str++;
        }
    }

    return -1;
}

int panoScriptGetPanoOutputSaveCoordinates(const pt_script* const script)
{
    assert((script != NULL) && (panoScriptGetPanoOutputFormat(script) > 1) && (panoScriptGetPanoOutputFormat(script) < 4));

    const char* str = script->pano.outputFormat;

    while (str != NULL)
    {
        str = strchr(str, ' ');

        if (str != NULL)
        {
            if (str[1] == 'p')
            {
                if (str[2] == '1')
                {
                    return 1;
                }

                return 0;
            }

            str++;
        }
    }

    return 0;
}

int panoScriptGetPanoOutputCropped(const pt_script* const script)
{
    assert((script != NULL) && (panoScriptGetPanoOutputFormat(script) > 1) && (panoScriptGetPanoOutputFormat(script) < 4));

    const char* str = script->pano.outputFormat;

    while (str != NULL)
    {
        str = strchr(str, ' ');

        if (str != NULL)
        {
            if (str[1] == 'r')
            {
                if (strncmp(":CROP", str + 2, 5) == 0)
                {
                    return 1;
                }

                return 0;
            }

            str++;
        }
    }

    return 0;
}

int panoScriptGetPanoOutputQuality(const pt_script* const script)
{
    assert((script != NULL) && (panoScriptGetPanoOutputFormat(script) == 4));

    char* str = script->pano.outputFormat;

    while (str != NULL)
    {
        str = strchr(str, ' ');

        if (str != NULL)
        {
            if (str[1] == 'q')
            {
                char* last = 0;
                int q      = strtol(str + 3, &last, 10);

                if (last != str + 3)
                {
                    return q;
                }
                else
                {
                    return -1;
                }
            }

            str++;
        }
    }

    return -1;
}

int panoScriptGetPanoIsHDR(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.dynamicRangeMode;
}

int panoScriptGetPanoBitDepth(const pt_script* const script)
{
    assert(script != NULL);

    return (int) script->pano.bitDepthOutput;
}

double panoScriptGetPanoExposure(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.exposureValue;
}

int panoScriptGetPanoImageReference(const pt_script* const script)
{
    assert(script != NULL);

    return script->pano.iImagePhotometricReference;
}

int panoScriptGetOptimizePrevCommentsCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iOptimize_prevCommentsCount;
}

char* panoScriptGetOptimizeComment(const pt_script* const script, int c)
{
    assert((script != NULL) && (c < panoScriptGetOptimizePrevCommentsCount(script)));

    return script->optimize_prevComments[c];
}

double panoScriptGetOptimizeGamma(const pt_script* const script)
{
    assert(script != NULL);

    return script->optimize.fGamma;
}

int panoScriptGetOptimizeInterpolator(const pt_script* const script)
{
    assert(script != NULL);

    return script->optimize.interpolator;
}

int panoScriptGetOptimizeSpeedUp(const pt_script* const script)
{
    assert(script != NULL);

    return script->optimize.fastFT;
}

double panoScriptGetOptimizeHuberSigma(const pt_script* const script)
{
    assert(script != NULL);

    return script->optimize.huberEstimator;
}

double panoScriptGetOptimizePhotometricHuberSigma(const pt_script* const script)
{
    assert(script != NULL);

    return script->optimize.photometricHuberSigma;
}

int panoScriptGetVarsToOptimizeCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iVarsToOptimizeCount;
}

int panoScriptGetVarsToOptimizePrevCommentCount(const pt_script* const script, int v)
{
    assert(script != NULL);

    return script->iVarsToOptimize_prevCommentsCount[v];
}

char* panoScriptGetVarsToOptimizeComment(const pt_script* const script, int v, int c)
{
    assert(script != NULL);

    return script->varsToOptimize_prevComments[v][c];
}

int panoScriptGetVarsToOptimizeImageId(const pt_script* const script, int v)
{
    assert((script != NULL) && (v >= 0) && (v < script->iVarsToOptimizeCount));

    return script->varsToOptimize[v].varIndex;
}

int panoScriptGetVarsToOptimizeName(const pt_script* const script, int v)
{
    assert((script != NULL) && (v >= 0) && (v < script->iVarsToOptimizeCount));

    const char* var = script->varsToOptimize[v].varName;

    switch (var[0])
    {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        {
            return var[0] - 'a';
        }

        case 'v':
        {
            return 5;
        }

        case 'y':
        {
            return 6;
        }

        case 'p':
        {
            return 7;
        }

        case 'r':
        {
            return 8;
        }

        case 'E':
        {
            switch (var[1])
            {
                case 'e':
                {
                    return 9;
                }

                case 'r':
                {
                    return 10;
                }

                case 'b':
                {
                    return 11;
                }

                default:
                {
                    return 23;
                }
            }
        }

        case 'V':
        {
            switch(var[1])
            {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                {
                    return var[1] - 'a' + 12;
                }

                case 'x':
                case 'y':
                {
                    return var[1] - 'x' + 16;
                }

                default:
                {
                    return 23;
                }
            }
        }

        case 'R':
        {
            switch (var[1])
            {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                {
                    return var[1] - 'a' + 18;
                }

                default:
                {
                    return 23;
                }
            }
        }

        default:
        {
            return 23;
        }
    }
}

int panoScriptGetCtrlPointCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iCtrlPointsCount;
}

int panoScriptGetCtrlPointPrevCommentCount(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->iCtrlPoints_prevCommentsCount[cp];
}

char* panoScriptGetCtrlPointComment(const pt_script* const script, int cp, int c)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPoints_prevComments[cp][c];
}

int panoScriptGetCtrlPointImage1(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].iImage1;
}

int panoScriptGetCtrlPointImage2(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].iImage2;
}

double panoScriptGetCtrlPointX1(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].p1.x;
}

double panoScriptGetCtrlPointX2(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].p2.x;
}

double panoScriptGetCtrlPointY1(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].p1.y;
}

double panoScriptGetCtrlPointY2(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].p2.y;
}

int panoScriptGetCtrlPointType(const pt_script* const script, int cp)
{
    assert((script != NULL) && (cp >= 0) && (cp < panoScriptGetCtrlPointCount(script)));

    return script->ctrlPointsSpec[cp].type;
}

int panoScriptGetMaskCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iMasksCount;
}

int panoScriptGetMaskPrevCommentCount(const pt_script* const script, int m)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->iCtrlPoints_prevCommentsCount[m];
}

char* panoScriptGetMaskComment(const pt_script* const script, int m, int c)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks_prevComments[m][c];
}

int panoScriptGetMaskImage(const pt_script* const script, int m)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks[m]->iImage;
}

int panoScriptGetMaskType(const pt_script* const script, int m)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks[m]->type;
}

int panoScriptGetMaskPointCount(const pt_script* const script, int m)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks[m]->iPointsCount;
}

int panoScriptGetMaskPointX(const pt_script* const script, int m, int p)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks[m]->points[p].x;
}

int panoScriptGetMaskPointY(const pt_script* const script, int m, int p)
{
    assert((script != NULL) && (m >= 0) && (m < script->iMasksCount));

    return script->masks[m]->points[p].y;
}

int panoScriptGetEndingCommentCount(const pt_script* const script)
{
    assert(script != NULL);

    return script->iEndingCommentsCount;
}

char* panoScriptGetEndingComment(const pt_script* const script, int c)
{
    assert((script != NULL) && (c >= 0) && (c < script->iEndingCommentsCount));

    return script->endingComments[c];
}
