/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread object detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autotagsclassifierbase.h"

// Qt includes

#include <QFile>
#include <QTextStream>
#include <QUnhandledException>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

AutotagsClassifierBase::AutotagsClassifierBase(float _threshold, const QString& _predefinedClassFile)
    : MLClassifierFoundation(),
      predefinedClassFile   (_predefinedClassFile)
{
    threshold = _threshold;

    if (!this->loadTrainingData())
    {
        qCCritical(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierBase Failed to load predefined classes from file:"
                                               << predefinedClassFile;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        std::runtime_error e("Failed to load predefined classes");
        std::exception_ptr p = std::make_exception_ptr(e);
        QUnhandledException(p).raise();

#else

        QUnhandledException().raise();

#endif

    }
}

bool AutotagsClassifierBase::loadTrainingData()
{
    QFile classFile(predefinedClassFile);

    if (classFile.exists() && classFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&classFile);

        while (!stream.atEnd())
        {
            predefinedClasses.append(stream.readLine());
        }

        return true;
    }
    else
    {
        return false;
    }
}

QList<QString> AutotagsClassifierBase::getClassStrings(const QList<int>& labelList) const
{
    QList<QString> result;

    for (int label : labelList)
    {
        if ((label >= 0) && (label < predefinedClasses.size()))
        {
            result << predefinedClasses[label];
        }
    }

    return result;
}

} // namespace Digikam
