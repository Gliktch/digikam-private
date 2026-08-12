/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QListWidget>
#include <QListWidgetItem>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgThreadedFilter;

class DIGIKAM_EXPORT PreviewThreadWrapper : public QObject
{
    Q_OBJECT

public:

    explicit PreviewThreadWrapper(QObject* const parent = nullptr);
    ~PreviewThreadWrapper() override;

    void registerFilter(int id, DImgThreadedFilter* const filter);

    void startFilters();
    void stopFilters();

Q_SIGNALS:

    void signalFilterStarted(int);
    void signalFilterFinished(int, const QPixmap&);

private Q_SLOTS:

    void slotFilterStarted();
    void slotFilterFinished(bool success);
    void slotFilterProgress(int progress);

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------

class DIGIKAM_EXPORT PreviewListItem : public QListWidgetItem
{

public:

    explicit PreviewListItem(QListWidget* const parent = nullptr);
    ~PreviewListItem()    override;

    void setPixmap(const QPixmap& pix);

    void setId(int _id);
    int  id()       const;

    void setBusy(bool b);
    bool isBusy()   const;

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(PreviewListItem)
};

// -------------------------------------------------------------------

class DIGIKAM_EXPORT PreviewList : public QListWidget
{
    Q_OBJECT

public:

    explicit PreviewList(QWidget* const parent = nullptr);
    ~PreviewList()                            override;

    PreviewListItem* addItem(DImgThreadedFilter* const filter, const QString& txt, int id);

    void setCurrentId(int id);
    int  currentId()                    const;

    void startFilters();
    void stopFilters();

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotFilterStarted(int);
    void slotFilterFinished(int, const QPixmap&);

private:

    PreviewListItem* findItem(int id)   const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
