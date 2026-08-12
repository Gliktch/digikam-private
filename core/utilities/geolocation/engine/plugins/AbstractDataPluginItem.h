/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QHash>

// Local includes

#include "BillboardGraphicsItem.h"
#include "digikam_export.h"

class QAction;

namespace Marble
{

class AbstractDataPluginItemPrivate;

class DIGIKAM_EXPORT AbstractDataPluginItem : public QObject, public BillboardGraphicsItem
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(bool favorite READ isFavorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(bool sticky READ isSticky WRITE setSticky NOTIFY stickyChanged)

public:

    explicit AbstractDataPluginItem(QObject* parent = nullptr);
    ~AbstractDataPluginItem()                                         override;

    /**
     * Returns the item's tool tip.
     */
    QString toolTip()                                           const;

    /**
     * Set the tool tip for the item.
     */
    void setToolTip(const QString& toolTip);

    QString id()                                                const;
    void setId(const QString& id);

    bool isFavorite()                                           const;
    virtual void setFavorite(bool favorite);

    bool isSticky()                                             const;
    void setSticky(bool sticky);

    /**
     * @brief Set the settings of the item.
     * This is usually called automatically before painting. If you reimplement this it would be
     * useful to check for changes before copying.
     */
    virtual void setSettings(const QHash<QString, QVariant>& settings);

    /**
     * Returns the action of this specific item.
     */
    virtual QAction* action();

    virtual bool initialized()                                  const = 0;

    virtual void addDownloadedFile(const QString& url,
                                   const QString& type);

    virtual bool operator<(const AbstractDataPluginItem* other) const = 0;

    virtual QList<QAction*> actions();

Q_SIGNALS:

    void updated();
    void idChanged();
    void favoriteChanged(const QString& id, bool favorite);
    void stickyChanged();

public Q_SLOTS:

    void toggleFavorite();

private:

    friend class AbstractDataPluginModel;

    /**
     * Returning the angular resolution of the viewport when the item was added to it the last
     * time.
     */
    qreal addedAngularResolution()                              const;
    void setAddedAngularResolution(qreal resolution);

private:

    AbstractDataPluginItemPrivate* const d = nullptr;
};

} // Marble namespace
