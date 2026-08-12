/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select albums using a tab of folder views.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "album.h"

namespace Digikam
{

class AbstractCheckableAlbumModel;
class AlbumLabelsSearchHandler;

class AlbumSelectTabs : public QTabWidget
{
    Q_OBJECT

public:

    explicit AlbumSelectTabs(const QString& name, QWidget* const parent = nullptr);
    ~AlbumSelectTabs()                                      override;

    AlbumList selectedAlbums()                        const;
    void enableVirtualAlbums(bool flag = true);

    QList<AbstractCheckableAlbumModel*> albumModels() const;
    AlbumLabelsSearchHandler* albumLabelsHandler()    const;

Q_SIGNALS:

    void signalAlbumSelectionChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
