/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class AdvancedRenameManager;

class DIGIKAM_GUI_EXPORT RenameCustomizer : public QWidget
{
    Q_OBJECT

public:

    enum Case
    {
        NONE = 0,
        UPPER,
        LOWER
    };

public:

    explicit RenameCustomizer(QWidget* const parent, const QString& cameraTitle);
    ~RenameCustomizer() override;

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString newName(const QString& fileName) const;
    Case    changeCase() const;
    void    setChangeCase(Case val);

    int     startIndex() const;
    void    setStartIndex(int index);
    void    reset();

    void    setPreviewText(const QString& txt);

    AdvancedRenameManager* renameManager() const;

Q_SIGNALS:

    void signalChanged();

private:

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotRadioButtonClicked(int);
    void slotRenameOptionsChanged();
    void slotCustomRenameChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
