/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "libsinfodlg_p.h"

namespace Digikam
{

void LibsInfoDlg::populateOpenCV()
{
    QTreeWidgetItem* const opencvHead = new QTreeWidgetItem(listView(), QStringList() << i18nc("@item: opencv info", "OpenCV Configuration"));
    listView()->addTopLevelItem(opencvHead);

    // --- OpenCV::OpenCL features.

    try
    {
        if (!cv::ocl::haveOpenCL() || !cv::ocl::useOpenCL())
        {
            new QTreeWidgetItem(opencvHead, QStringList() <<
                                i18nc(Private::CONTEXT, "OpenCL availability") << d->supportedNo());
        }
        else
        {
            std::vector<cv::ocl::PlatformInfo> platforms;
            cv::ocl::getPlatfomsInfo(platforms);

            if (platforms.empty())
            {
                new QTreeWidgetItem(opencvHead, QStringList() <<
                                    i18nc(Private::CONTEXT, "OpenCL availability") << d->supportedNo());

            }
            else
            {
                QTreeWidgetItem* const oclplfrm = new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "OpenCL platform"));

                for (size_t i = 0 ; i < platforms.size() ; i++)
                {
                    const cv::ocl::PlatformInfo* platform = &platforms[i];

                    QTreeWidgetItem* const plfrm = new QTreeWidgetItem(oclplfrm, QStringList() << QString::fromStdString(platform->name()));

                    cv::ocl::Device current_device;

                    for (int j = 0 ; j < platform->deviceNumber() ; j++)
                    {
                        platform->getDevice(current_device, j);
                        QString deviceTypeStr = d->openCVGetDeviceTypeString(current_device);

                        new QTreeWidgetItem(plfrm, QStringList()
                            << deviceTypeStr << QString::fromStdString(current_device.name()) +
                               QLatin1String(" (") + QString::fromStdString(current_device.version()) + QLatin1Char(')'));
                    }

                    if (plfrm->childCount() == 0)
                    {
                        new QTreeWidgetItem(plfrm, QStringList() << i18n("no entry"));
                    }
                }

                if (oclplfrm->childCount() == 0)
                {
                    new QTreeWidgetItem(oclplfrm, QStringList() << i18n("no entry"));
                }

                const cv::ocl::Device& device = cv::ocl::Device::getDefault();

                if (!device.available())
                {
                    new QTreeWidgetItem(opencvHead, QStringList() <<
                                        i18nc(Private::CONTEXT, "OpenCL device") << d->supportedNo());
                }
                else
                {
                    QTreeWidgetItem* const ocldev = new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "OpenCL Device"));
                    QString deviceTypeStr         = d->openCVGetDeviceTypeString(device);

                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Type")                       << deviceTypeStr);
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Name")                       << QString::fromStdString(device.name()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Version")                    << QString::fromStdString(device.version()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Driver version")             << QString::fromStdString(device.driverVersion()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Address bits")               << QString::number(device.addressBits()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Compute units")              << QString::number(device.maxComputeUnits()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Max work group size")        << QString::number(device.maxWorkGroupSize()));

                    QString localMemorySizeStr = d->openCVBytesToStringRepr(device.localMemSize());
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Local memory size")          << localMemorySizeStr);

                    QString maxMemAllocSizeStr = d->openCVBytesToStringRepr(device.maxMemAllocSize());
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Max memory allocation size") << maxMemAllocSizeStr);

                    QString doubleSupportStr = (device.doubleFPConfig() > 0) ? d->supportedYes() : d->supportedNo();
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Double support")             << doubleSupportStr);

                    QString halfSupportStr = (device.halfFPConfig() > 0) ? d->supportedYes() : d->supportedNo();
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Half support")               << halfSupportStr);

                    QString isUnifiedMemoryStr = device.hostUnifiedMemory() ? d->supportedYes() : d->supportedNo();
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Host unified memory")        << isUnifiedMemoryStr);

                    QString haveAmdBlasStr = cv::ocl::haveAmdBlas() ? d->supportedYes() : d->supportedNo();
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Has AMD Blas")                  << haveAmdBlasStr);

                    QString haveAmdFftStr  = cv::ocl::haveAmdFft() ? d->supportedYes() : d->supportedNo();
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Has AMD Fft")                   << haveAmdFftStr);
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width char")   << QString::number(device.preferredVectorWidthChar()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width short")  << QString::number(device.preferredVectorWidthShort()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width int")    << QString::number(device.preferredVectorWidthInt()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width long")   << QString::number(device.preferredVectorWidthLong()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width float")  << QString::number(device.preferredVectorWidthFloat()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width double") << QString::number(device.preferredVectorWidthDouble()));
                    new QTreeWidgetItem(ocldev, QStringList() << i18nc(Private::CONTEXT, "Preferred vector width half")   << QString::number(device.preferredVectorWidthHalf()));

                    QTreeWidgetItem* const ocldevext = new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "OpenCL Device extensions"));
                    QString extensionsStr            = QString::fromStdString(device.extensions());
                    int pos                          = 0;

                    while (pos < extensionsStr.size())
                    {
                        int pos2 = extensionsStr.indexOf(QLatin1Char(' '), pos);

                        if (pos2 == -1)
                        {
                            pos2 = extensionsStr.size();
                        }

                        if (pos2 > pos)
                        {
                            QString extensionName = extensionsStr.mid(pos, pos2 - pos);
                            new QTreeWidgetItem(ocldevext, QStringList() << extensionName << d->supportedYes());
                        }

                        pos = pos2 + 1;
                    }

                    if (ocldevext->childCount() == 0)
                    {
                        new QTreeWidgetItem(ocldevext, QStringList() << i18n("no entry"));
                    }
                }
            }
        }
    }
    catch (...)
    {
        new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "OpenCL availability") << d->supportedNo());
    }

    // --- OpenCV::Hardware features.

    try
    {
        QTreeWidgetItem* const ocvhdw = new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "Hardware features"));
        int count                     = 0;
        Q_UNUSED(count);

        for (int i = 0 ; i < CV_HARDWARE_MAX_FEATURE ; i++)
        {
            QString name = QString::fromStdString(cv::getHardwareFeatureName(i));

            if (name.isEmpty())
            {
                continue;
            }

            bool enabled = cv::checkHardwareSupport(i);

            if (enabled)
            {
                count++;
                new QTreeWidgetItem(ocvhdw, QStringList() << name << d->supportedYes());
            }
        }
    }
    catch (...)
    {
        new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "Hardware features availability") << d->supportedNo());
    }

    // --- OpenCV::Threads features.

    try
    {
        QTreeWidgetItem* const ocvthreads = new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "Threads features"));

        QString parallelFramework = QString::fromStdString(cv::currentParallelFramework());

        if (!parallelFramework.isEmpty())
        {
            new QTreeWidgetItem(ocvthreads, QStringList() << i18nc(Private::CONTEXT, "Number of Threads") << QString::number(cv::getNumThreads()));
            new QTreeWidgetItem(ocvthreads, QStringList() << i18nc(Private::CONTEXT, "Parallel framework") << parallelFramework);
        }
    }
    catch (...)
    {
        new QTreeWidgetItem(opencvHead, QStringList() << i18nc(Private::CONTEXT, "Threads features availability") << d->supportedNo());
    }
}

} // namespace Digikam
