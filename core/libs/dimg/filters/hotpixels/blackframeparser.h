/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : black frames parser
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * Part of the algorithm for finding the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * written by Martin Dickopp
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QObject>
#include <QString>
#include <QRect>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "hotpixelprops.h"
#include "loadingdescription.h"

using namespace Digikam;

namespace Digikam
{

class DIGIKAM_EXPORT BlackFrameParser: public QObject
{
    Q_OBJECT

public:

    explicit BlackFrameParser(QObject* const parent);
    ~BlackFrameParser() override;

    void parseHotPixels(const QString& file);
    void parseBlackFrame(const QUrl& url);
    void parseBlackFrame(const DImg& img);

    DImg image() const;

Q_SIGNALS:

    void signalHotPixelsParsed(const QList<HotPixelProps>&);
    void signalLoadingProgress(float);
    void signalLoadingComplete();

private Q_SLOTS:

    void slotLoadingProgress(const LoadingDescription&, float v);
    void slotLoadImageFromUrlComplete(const LoadingDescription&, const DImg& img);

private:

    void blackFrameParsing();
    void consolidatePixels(QList<HotPixelProps>& list);
    void validateAndConsolidate(HotPixelProps* const a, HotPixelProps* const b);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
