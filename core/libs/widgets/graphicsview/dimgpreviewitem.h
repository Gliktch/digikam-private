/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View items for DImg
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGraphicsItem>
#include <QObject>

// Local includes

#include "digikam_export.h"
#include "graphicsdimgitem.h"

namespace Digikam
{

class DImg;
class ICCSettingsContainer;
class LoadingDescription;
class PreviewSettings;

class DIGIKAM_EXPORT DImgPreviewItem : public GraphicsDImgItem
{
    Q_OBJECT

public:

    enum State
    {
        NoImage,
        Loading,
        ImageLoaded,
        ImageLoadingFailed
    };

public:

    explicit DImgPreviewItem(QGraphicsItem* const parent = nullptr);
    ~DImgPreviewItem()                override;

    void setDisplayingWidget(QWidget* const widget);
    void setPreviewSettings(const PreviewSettings& settings);

    QString path()              const;
    void    setPath(const QString& path, bool rePreview = false);

    State state()               const;
    bool  isLoaded()            const;
    void  reload();

    void setPreloadPaths(const QStringList& pathsToPreload);

    QString userLoadingHint()   const override;

Q_SIGNALS:

    /**
     * @note Used for the progress bar on the top level.
     */
    void signalStartedLoading();
    void signalLoadingProgress(float progress);
    void signalLoadingComplete();

    void stateChanged(int state);
    void loaded();
    void loadingFailed();

private Q_SLOTS:

    void slotLoadingProgress(const LoadingDescription& loadingDescription, float progress);
    void slotGotImagePreview(const LoadingDescription& loadingDescription, const DImg& image);
    void preloadNext();
    void slotFileChanged(const QString& path);
    void iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous);

private:

    // @note disabled
    DImgPreviewItem(QObject*);

    class DImgPreviewItemPrivate;
    Q_DECLARE_PRIVATE(DImgPreviewItem)          // cppcheck-suppress duplInheritedMember

protected:

    explicit DImgPreviewItem(DImgPreviewItemPrivate& dd, QGraphicsItem* const parent = nullptr);
};

} // namespace Digikam
