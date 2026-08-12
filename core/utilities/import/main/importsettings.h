/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-15
 * Description : Settings for the import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QFont>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImportSettings : public QObject
{
    Q_OBJECT

public:

    enum ItemLeftClickAction
    {
        ShowPreview = 0,
        StartEditor,
        OpenDefault
    };

Q_SIGNALS:

    void setupChanged();

public:

    ImportSettings();
    ~ImportSettings()                             override;

    static ImportSettings* instance();

    void readSettings();
    void saveSettings();

    void emitSetupChanged();

    bool showToolTipsIsValid()              const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar()                  const;

    void setPreviewLoadFullImageSize(bool val);
    bool getPreviewLoadFullImageSize()      const;

    void setPreviewItemsWhileDownload(bool val);
    bool getPreviewItemsWhileDownload()     const;

    void setPreviewShowIcons(bool val);
    bool getPreviewShowIcons()              const;

    void setImageSortOrder(int order);
    int  getImageSortOrder()                const;

    void setImageSortBy(int sortBy);
    int  getImageSortBy()                   const;

    void setImageSeparationMode(int mode);
    int  getImageSeparationMode()           const;

    void setItemLeftClickAction(int action);
    int  getItemLeftClickAction()           const;

    void setDefaultIconSize(int val);
    int  getDefaultIconSize()               const;

    void  setIconViewFont(const QFont& font);
    QFont getIconViewFont()                 const;

    void setIconShowName(bool val);
    bool getIconShowName()                  const;

    void setIconShowSize(bool val);
    bool getIconShowSize()                  const;

    void setIconShowTitle(bool val);
    bool getIconShowTitle()                 const;

    void setIconShowTags(bool val);
    bool getIconShowTags()                  const;

    void setIconShowDate(bool val);
    bool getIconShowDate()                  const;

    void setIconShowRating(bool val);
    bool getIconShowRating()                const;

    void setIconShowImageFormat(bool val);
    bool getIconShowImageFormat()           const;

    void setIconShowCoordinates(bool val);
    bool getIconShowCoordinates()           const;

    void setIconShowOverlays(bool val);
    bool getIconShowOverlays()              const;

    void  setToolTipsFont(const QFont& font);
    QFont getToolTipsFont()                 const;

    void setShowToolTips(bool val);
    bool getShowToolTips()                  const;

    void setToolTipsShowFileName(bool val);
    bool getToolTipsShowFileName()          const;

    void setToolTipsShowFileDate(bool val);
    bool getToolTipsShowFileDate()          const;

    void setToolTipsShowFileSize(bool val);
    bool getToolTipsShowFileSize()          const;

    void setToolTipsShowImageType(bool val);
    bool getToolTipsShowImageType()         const;

    void setToolTipsShowImageDim(bool val);
    bool getToolTipsShowImageDim()          const;

    void setToolTipsShowPhotoMake(bool val);
    bool getToolTipsShowPhotoMake()         const;

    void setToolTipsShowPhotoLens(bool val);
    bool getToolTipsShowPhotoLens()         const;

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal()        const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo()         const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash()        const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB()           const;

    void setToolTipsShowTags(bool val);
    bool getToolTipsShowTags()              const;

    void setToolTipsShowLabelRating(bool val);
    bool getToolTipsShowLabelRating()       const;

private:

    /// @note disabled;
    explicit ImportSettings(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
