/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A working pixmap manager.
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QSize>
#include <QPixmap>
#include <QObject>
#include <QVector>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** A widget to draw progress wheel indicator over thumbnails.
 */
class DIGIKAM_EXPORT DWorkingPixmap : public QObject
{
    Q_OBJECT

public:

    explicit DWorkingPixmap(QObject* const parent = nullptr);
    ~DWorkingPixmap() override = default;

    bool    isEmpty()          const;
    QSize   frameSize()        const;
    int     frameCount()       const;
    QPixmap frameAt(int index) const;

private:

    QVector<QPixmap> m_frames;
};

} // namespace Digikam
