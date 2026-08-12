/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on smoothslidesaver by Carsten Weinhold
 * <carsten dot weinhold at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QImage>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QPair>
#include <QString>

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class KBImageLoader : public QThread
{
    Q_OBJECT

public:

    KBImageLoader(PresentationContainer* const sharedData, int width, int height);
    ~KBImageLoader()                  override;

    void  quit();
    void  requestNewImage();

    void          ungrabImage();
    bool          grabImage();
    bool          ready()       const;
    const QImage& image()       const;
    float         imageAspect() const;

protected:

    void run()                        override;
    bool loadImage();
    void invalidateCurrentImageName();

private:

    /// @note disabled
    explicit KBImageLoader(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
