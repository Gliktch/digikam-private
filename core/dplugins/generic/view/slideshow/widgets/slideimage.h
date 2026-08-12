/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow image widget
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "loadingdescription.h"
#include "previewsettings.h"
#include "slideshowsettings.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class SlideImage : public QWidget
{
    Q_OBJECT

public:

    explicit SlideImage(QWidget* const parent);
    ~SlideImage()                   override;

    void setSlideShowSettings(SlideShowSettings* const settings);
    void setLoadUrl(const QUrl& url);
    void setPreloadUrl(const QUrl& url);

Q_SIGNALS:

    void signalImageLoaded(bool);

private Q_SLOTS:

    void slotGotImagePreview(const LoadingDescription&, const DImg&);

private:

    void paintEvent(QPaintEvent*)   override;
    void updatePixmap();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSlideShowPlugin
