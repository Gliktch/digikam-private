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

#pragma once

// Qt includes

#include <QString>

namespace DigikamGenericGLViewerPlugin
{

class GLViewerTimer
{

public:

    GLViewerTimer();
    ~GLViewerTimer();

    void start();
    void at(const QString& s);

private:

    /// @note disabled
    GLViewerTimer(const GLViewerTimer&)            = delete;
    GLViewerTimer& operator=(const GLViewerTimer&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGLViewerPlugin
