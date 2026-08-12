/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for Showfoto
 *
 * SPDX-FileCopyrightText: 2013-2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QFont>
#include <QString>

namespace ShowFoto
{

class ShowfotoSettings : public QObject
{
    Q_OBJECT

public:

    ShowfotoSettings();
    ~ShowfotoSettings() override;

    static ShowfotoSettings* instance();

    void readSettings();
    void syncConfig();

    // -- Misc. Settings ---------------------------------------

    QString getLastOpenedDir() const;
    void setLastOpenedDir(const QString& dir);

    bool getDeleteItem2Trash() const;
    void setDeleteItem2Trash(bool D2t);

    QString getCurrentTheme() const;
    void setCurrentTheme(const QString& theme);

    int getRightSideBarStyle() const;
    void setRightSideBarStyle(int style);

    int getUpdateType() const;
    void setUpdateType(int type);

    bool getUpdateWithDebug() const;
    void setUpdateWithDebug(bool dbg);

    QString getApplicationStyle() const;
    void setApplicationStyle(const QString& style);

    QString getIconTheme() const;
    void setIconTheme(const QString& theme);

    QString getDateTimeFormat() const;
    void setDateTimeFormat(const QString& fmt);

    void setApplicationFont(const QFont& fnt);
    QFont getApplicationFont() const;

    bool getShowFormatOverThumbnail() const;
    void setShowFormatOverThumbnail(bool show);

    bool getShowCoordinates() const;
    void setShowCoordinates(bool show);

    bool getShowSplash() const;
    void setShowSplash(bool show);

    bool getNativeFileDialog() const;
    void setNativeFileDialog(bool item);

    bool getItemCenter() const;
    void setItemCenter(bool item);

    bool getCacheThumbs() const;
    void setCacheThumbs(bool item);

    int  getSortRole() const;
    void setSortRole(int order);

    bool getReverseSort() const;
    void setReverseSort(bool reverse);

    // -- ToolTip Settings --------------------------------------

    bool getShowToolTip() const;
    void setShowToolTip(bool show);

    bool getShowFileName() const;
    void setShowFileName(bool show);

    bool getShowFileDate() const;
    void setShowFileDate(bool show);

    bool getShowFileSize() const;
    void setShowFileSize(bool show);

    bool getShowFileType() const;
    void setShowFileType(bool show);

    bool getShowFileDim() const;
    void setShowFileDim(bool show);

    bool getShowPhotoMake() const;
    void setShowPhotoMake(bool show);

    bool getShowPhotoLens() const;
    void setShowPhotoLens(bool show);

    bool getShowPhotoFocal() const;
    void setShowPhotoFocal(bool show);

    bool getShowPhotoExpo() const;
    void setShowPhotoExpo(bool show);

    bool getShowPhotoFlash() const;
    void setShowPhotoFlash(bool show);

    bool getShowPhotoWB() const;
    void setShowPhotoWB(bool show);

    bool getShowPhotoDate() const;
    void setShowPhotoDate(bool show);

    bool getShowPhotoMode() const;
    void setShowPhotoMode(bool show);

    QFont getToolTipFont() const;
    void setToolTipFont(const QFont& font);

private:

    /// @note disabled
    explicit ShowfotoSettings(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Showfoto
