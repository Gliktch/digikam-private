/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelNet base class
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelinfocontainer.h"

// Qt Includes

#include <QFileInfo>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "dnnmodelmanager.h"

namespace Digikam
{

DNNModelInfoContainer::DNNModelInfoContainer(const DNNModelInfoContainer& o)
    : displayName       (o.displayName),
      toolTip           (o.toolTip),
      fileName          (o.fileName),
      usage             (o.usage),
      minVersion        (o.minVersion),
      downloadPath      (o.downloadPath),
      sha256            (o.sha256),
      preprocessor      (o.preprocessor),
      classifier        (o.classifier),
      classList         (o.classList),
      configName        (o.configName),
      fileSize          (o.fileSize),
      defaultThreshold  (o.defaultThreshold),
      minUsableThreshold(o.minUsableThreshold),
      maxUsableThreshold(o.maxUsableThreshold),
      loaderType        (o.loaderType),
      meanValToSubtract (o.meanValToSubtract),
      imageSize         (o.imageSize)
{
}

DNNModelInfoContainer& DNNModelInfoContainer::operator=(const DNNModelInfoContainer& o)
{
    displayName        = o.displayName;
    toolTip            = o.toolTip;
    fileName           = o.fileName;
    usage              = o.usage;
    minVersion         = o.minVersion;
    downloadPath       = o.downloadPath;
    sha256             = o.sha256;
    preprocessor       = o.preprocessor;
    classifier         = o.classifier;
    classList          = o.classList;
    configName         = o.configName;
    fileSize           = o.fileSize;
    defaultThreshold   = o.defaultThreshold;
    minUsableThreshold = o.minUsableThreshold;
    maxUsableThreshold = o.maxUsableThreshold;
    loaderType         = o.loaderType;
    meanValToSubtract  = o.meanValToSubtract;
    imageSize          = o.imageSize;

    return *this;
}

DNNModelInfoContainer& DNNModelInfoContainer::operator=(DNNModelInfoContainer&& o)
{
    displayName        = o.displayName;
    toolTip            = o.toolTip;
    fileName           = o.fileName;
    usage              = o.usage;
    minVersion         = o.minVersion;
    downloadPath       = o.downloadPath;
    sha256             = o.sha256;
    preprocessor       = o.preprocessor;
    classifier         = o.classifier;
    classList          = o.classList;
    configName         = o.configName;
    fileSize           = o.fileSize;
    defaultThreshold   = o.defaultThreshold;
    minUsableThreshold = o.minUsableThreshold;
    maxUsableThreshold = o.maxUsableThreshold;
    loaderType         = o.loaderType;
    meanValToSubtract  = o.meanValToSubtract;
    imageSize          = o.imageSize;

    return *this;
}

DNNModelInfoContainer::DNNModelInfoContainer(
                                             const QString&           _displayName,
                                             const QString&           _toolTip,
                                             const QString&           _fileName,
                                             const DNNModelUsageList& _usage,
                                             const QVersionNumber&    _minVersion,
                                             const QString&           _downloadPath,
                                             const QString&           _sha256,
                                             const qint64&            _fileSize,
                                             int                      _defaultThreshold,
                                             int                      _minUsableThreshold,
                                             int                      _maxUsableThreshold,
                                             DNNLoaderType            _loaderType,
                                             const QString&           _preprocessor,
                                             const QString&           _classifier,
                                             const QString&           _classList,
                                             const QString&           _configName,
                                             const cv::Scalar&        _meanValToSubtract,
                                             int                      _imageSize
                                            )
    : displayName       (_displayName),
      toolTip           (_toolTip),
      fileName          (_fileName),
      usage             (_usage),
      minVersion        (_minVersion),
      downloadPath      (_downloadPath),
      sha256            (_sha256),
      preprocessor      (_preprocessor),
      classifier        (_classifier),
      classList         (_classList),
      configName        (_configName),
      fileSize          (_fileSize),
      defaultThreshold  (_defaultThreshold),
      minUsableThreshold(_minUsableThreshold),
      maxUsableThreshold(_maxUsableThreshold),
      loaderType        (_loaderType),
      meanValToSubtract (_meanValToSubtract),
      imageSize         (_imageSize)
{
}

bool DNNModelInfoContainer::operator==(const DNNModelInfoContainer& o) const
{
    bool b = true;
    b     &= (displayName         == o.displayName);
    b     &= (toolTip             == o.toolTip);
    b     &= (fileName            == o.fileName);
    b     &= (usage               == o.usage);
    b     &= (minVersion          == o.minVersion);
    b     &= (downloadPath        == o.downloadPath);
    b     &= (sha256              == o.sha256);
    b     &= (preprocessor        == o.preprocessor);
    b     &= (classifier          == o.classifier);
    b     &= (classList           == o.classList);
    b     &= (configName          == o.configName);
    b     &= (fileSize            == o.fileSize);
    b     &= (defaultThreshold    == o.defaultThreshold);
    b     &= (minUsableThreshold  == o.minUsableThreshold);
    b     &= (maxUsableThreshold  == o.maxUsableThreshold);
    b     &= (loaderType          == o.loaderType);
    b     &= (imageSize           == o.imageSize);

    return b;
}

} // namespace Digikam
