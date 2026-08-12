/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-11
 * Description : a widget to customize album name created by
 *               camera interface.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AlbumCustomizer : public QWidget
{
    Q_OBJECT

public:

    enum DateFormatOptions
    {
        IsoDateFormat = 0,
        TextDateFormat,
        LocalDateFormat,
        CustomDateFormat
    };

public:

    explicit AlbumCustomizer(QWidget* const parent = nullptr);
    ~AlbumCustomizer()                      override;

    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    bool    autoAlbumDateEnabled()    const;
    bool    autoAlbumExtEnabled()     const;
    int     folderDateFormat()        const;
    bool    customDateFormatIsValid() const;
    QString customDateFormat()        const;

private Q_SLOTS:

    void slotFolderDateFormatChanged(int);
    void slotToolTipButtonToggled(bool);
    void slotCustomizerChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
