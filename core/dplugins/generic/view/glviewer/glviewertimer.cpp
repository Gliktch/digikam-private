/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "glviewertimer.h"

// Qt includes

#include <QDateTime>
#include <QElapsedTimer>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericGLViewerPlugin
{

class Q_DECL_HIDDEN GLViewerTimer::Private
{
public:

    Private() = default;

    QElapsedTimer timer;
    int           meantime = 0;
};

GLViewerTimer::GLViewerTimer()
    : d(new Private)
{
}

GLViewerTimer::~GLViewerTimer()
{
    delete d;
}

void GLViewerTimer::start()
{
    d->timer.start();
    d->meantime = 0;
}

void GLViewerTimer::at(const QString& s)
{
    d->meantime = d->timer.elapsed() - d->meantime;

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "stopwatch:" << s
                                         << ": " << d->meantime
                                         << " ms    overall: "
                                         << d->timer.elapsed() << " ms";
}

} // namespace DigikamGenericGLViewerPlugin
