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

#include "dworkingpixmap.h"

// Qt includes

#include <QApplication>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DWorkingPixmap::DWorkingPixmap(QObject* const parent)
    : QObject(parent)
{
    QPixmap pix(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                       QLatin1String("digikam/data/process-working.png")));
    if (pix.isNull())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid pixmap specified.";

        return;
    }

    QSize size = QSize(pix.width(), pix.width());

    if (((pix.width()) % size.width()) || (pix.height() % size.height()))
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid framesize.";

        return;
    }

    const int rowCount = pix.height() / size.height();
    const int colCount = pix.width()  / size.width();
    m_frames.resize(rowCount * colCount);

    int pos = 0;

    for (int row = 0 ; row < rowCount ; ++row)
    {
        for (int col = 0 ; col < colCount ; ++col)
        {
            QPixmap frm     = pix.copy(col * size.width(), row * size.height(), size.width(), size.height());
            m_frames[pos++] = frm;
        }
    }
}

bool DWorkingPixmap::isEmpty() const
{
    return m_frames.isEmpty();
}

QSize DWorkingPixmap::frameSize() const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";

        return QSize();
    }

    return m_frames[0].size();
}

int DWorkingPixmap::frameCount() const
{
    return m_frames.size();
}

QPixmap DWorkingPixmap::frameAt(int index) const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";

        return QPixmap();
    }

    return m_frames.at(index);
}

} // namespace Digikam

#include "moc_dworkingpixmap.cpp"
