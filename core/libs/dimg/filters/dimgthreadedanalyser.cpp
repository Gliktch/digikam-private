/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image analys class.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgthreadedanalyser.h"

namespace Digikam
{

DImgThreadedAnalyser::DImgThreadedAnalyser(QObject* const parent, const QString& name)
    : DImgThreadedFilter(parent, name)
{
}

DImgThreadedAnalyser::DImgThreadedAnalyser(DImg* const orgImage, QObject* const parent,
                                           const QString& name)
    : DImgThreadedFilter(orgImage, parent, name)
{
}

DImgThreadedAnalyser::~DImgThreadedAnalyser()
{
}

} // namespace Digikam

#include "moc_dimgthreadedanalyser.cpp"
