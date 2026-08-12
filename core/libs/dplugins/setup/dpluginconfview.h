/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QTreeWidgetItem>
#include <QTreeWidget>

// Local includes

#include "dpluginloader.h"
#include "digikam_export.h"
#include "dpluginaction.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginConfView : public QTreeWidget
{
    Q_OBJECT

public:

    enum Columns
    {
        Name            = 0,
        Categories      = 1,
        Tools           = 2,
        Description     = 3,

        NumberOfColumns
    };

public:

    /**
     * Default constructor.
     */
    explicit DPluginConfView(QWidget* const parent = nullptr);
    ~DPluginConfView()                                 override;

    /**
     * Apply all changes about plugins selected to be hosted in host application.
     */
    void apply();

    /**
     * Return the number of plugins active in the list.
     */
    int activated()                              const;

    /**
     * Return the total number of plugins in the list.
     */
    int count()                                  const;

    /**
     * Return the number of visible plugins in the list.
     */
    int itemsVisible()                           const;

    /**
     * Return the number of plugins in the list with visibly properties available.
     */
    int itemsWithVisiblyProperty()               const;

    /**
     * Select all plugins in the list.
     */
    void selectAll()                                   override;

    /**
     * Clear all selected plugins in the list.
     */
    void clearAll();

    /**
     * Set the string used to filter the plugins list. signalSearchResult() is emitted when all is done.
     */
    void setFilter(const QString& filter, Qt::CaseSensitivity cs);

    /**
     * Return the current string used to filter the plugins list.
     */
    QString filter()                             const;

    DPlugin* plugin(QTreeWidgetItem* const item) const;

protected:

    QTreeWidgetItem* appendPlugin(DPlugin* const);

    virtual void loadPlugins() = 0;

Q_SIGNALS:

    /**
     * Signal emitted when filtering is done through slotSetFilter().
     * Number of plugins found is sent when item relevant of filtering match the query.
     */
    void signalSearchResult(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
