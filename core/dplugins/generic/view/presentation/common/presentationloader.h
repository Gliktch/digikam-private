/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-11
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QImage>
#include <QUrl>

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationLoader
{
public:

    explicit PresentationLoader(PresentationContainer* const sharedData,
                                int width,
                                int height,
                                int beginAtIndex = 0);

    ~PresentationLoader();

    void    next();
    void    prev();

    QImage  getCurrent()        const;
    QString currFileName()      const;
    QUrl    currPath()          const;

private:

    void checkIsIn(int index)   const;

private:

    /// @note disabled
    PresentationLoader(const PresentationLoader&)            = delete;
    PresentationLoader& operator=(const PresentationLoader&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
