/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Batch Queue Manager digiKam plugin definition.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "dplugin.h"
#include "dpluginloader.h"
#include "batchtool.h"
#include "bqminfoiface.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT DPluginBqm : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginBqm(QObject* const parent = nullptr);

    /**
     * Destructor
     */
    ~DPluginBqm()                                             override;

public:

    bool hasVisibilityProperty()                        const override;

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    void setVisible(bool b)                                   override;

    /**
     * Return all plugin tools registered in setup() method with addTool() for a given parent.
     */
    QList<BatchTool*> tools(QObject* const parent)      const;

    /**
     * Return the amount of tools registered.
     */
    int count()                                         const override;

    /**
     * Return a plugin tool instance found by name in plugin tools list for a given parent.
     */
    BatchTool* findToolByName(const QString& name,
                              QObject* const parent)    const;

    /**
     * Return a list of batch tool group categories as strings registered in this plugin.
     */
    QStringList categories()                            const override;

    /**
     * Return the plugin interface identifier.
     */
    QString ifaceIid()                                  const override;

Q_SIGNALS:

    void signalVisible(bool);

protected:

    void addTool(BatchTool* const t);

public:

    /**
     * Return the info interface instance.
     */
    BqmInfoIface* infoIface()                           const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginBqm, DIGIKAM_DPLUGIN_BQM_IID)
