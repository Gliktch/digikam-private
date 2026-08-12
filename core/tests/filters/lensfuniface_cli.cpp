/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : a command line tool to parse metadata dedicated to LensFun
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QFile>
#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "lensfuniface.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "testlensfuniface - test to parse metadata dedicated to LensFun";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QString filePath(QString::fromUtf8(argv[1]));
    bool valRet = false;

    DImg         img(filePath);
    QScopedPointer<DMetadata> meta(new DMetadata(img.getMetadata()));
    LensFunIface iface;
    LensFunIface::MetadataMatch ret = iface.findFromMetadata(meta.data());

    if (ret == LensFunIface::MetadataExactMatch)
    {
        LensFunFilter filter(&img, nullptr, iface.settings());
        filter.startFilterDirectly();
        img.putImageData(filter.getTargetImage().bits());

        Digikam::MetaEngineData data = img.getMetadata();
        filter.registerSettingsToXmp(data);
        img.setMetadata(data);
        img.save(QLatin1String("lensfuniface-output.png"), QLatin1String("PNG"));

        valRet = true;
    }

    return valRet;
}
