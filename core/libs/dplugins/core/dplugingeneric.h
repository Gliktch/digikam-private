/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Generic digiKam plugin definition.
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

#include "dinfointerface.h"
#include "dplugin.h"
#include "dpluginaction.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginGeneric : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginGeneric(QObject* const parent = nullptr);

    /**
     * Destructor
     */
    ~DPluginGeneric() override;

public:

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    void setVisible(bool b) override;

    /**
     * Return all plugin actions registered in setup() method with addAction() for a given parent.
     */
    QList<DPluginAction*> actions(QObject* const parent) const;

    /**
     * Return the amount of tools registered to all parents.
     */
    int count() const override;

    /**
     * Return a plugin action instance found by name in plugin action list for a given parent.
     */
    DPluginAction* findActionByName(const QString& name, QObject* const parent) const;

    /**
     * Return a list of categories as strings registered in this plugin.
     */
    QStringList categories() const override;

    /**
     * Return the plugin interface identifier.
     */
    QString ifaceIid() const override { return QLatin1String(DIGIKAM_DPLUGIN_GENERIC_IID); };

protected:

    void addAction(DPluginAction* const ac);

    /**
     * Return the info interface instance for the given action object.
     */
    DInfoInterface* infoIface(QObject* const ac) const;

    /**
     * Helper function to reactivate the desktop visibility of tool widget.
     */
    bool reactivateToolDialog(QWidget* const dlg) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginGeneric, DIGIKAM_DPLUGIN_GENERIC_IID)
