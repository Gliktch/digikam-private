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

const char* LibsInfoDlg::Private::CONTEXT = "@item: Component information, see help->components";

QString LibsInfoDlg::Private::checkTriState(int value) const
{
    switch (value)
    {
        case true:
        {
            return i18nc("@info: tri state", "Yes");
        }

        case false:
        {
            return i18nc("@info: tri state", "No");
        }

        default:
        {
            return i18nc("@info: tri state", "Unknown");
        }
    }
}

QString LibsInfoDlg::Private::supportedNo() const
{
    return i18nc("@item: component is not available/supported", "No");
}

QString LibsInfoDlg::Private::supportedYes() const
{
    return i18nc("@item: component is supported/available",     "Yes");
}

QString LibsInfoDlg::Private::openCVBytesToStringRepr(size_t value) const
{
    size_t b  = value % 1024;
    value    /= 1024;

    size_t kb = value % 1024;
    value    /= 1024;

    size_t mb = value % 1024;
    value    /= 1024;

    size_t gb = value;

    QString s;
    QTextStream stream(&s);

    if (gb > 0)
    {
        stream << gb << " GB ";
    }

    if (mb > 0)
    {
        stream << mb << " MB ";
    }

    if (kb > 0)
    {
        stream << kb << " KB ";
    }

    if (b > 0)
    {
        stream << b << " B";
    }

    if (s[s.size() - 1] == QLatin1Char(' '))
    {
        s = s.mid(0, s.size() - 1);
    }

    return s;
}

QString LibsInfoDlg::Private::openCVGetDeviceTypeString(const cv::ocl::Device& device)
{
    if (device.type() == cv::ocl::Device::TYPE_CPU)
    {
        return QLatin1String("CPU");
    }

    if (device.type() == cv::ocl::Device::TYPE_GPU)
    {
        if (device.hostUnifiedMemory())
        {
            return QLatin1String("iGPU");
        }
        else
        {
            return QLatin1String("dGPU");
        }
    }

    return QLatin1String("unknown");
}

} // namespace Digikam
