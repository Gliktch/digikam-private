/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Image Editor digiKam plugin definition.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dplugineditor.h"

// Qt includes

#include <QApplication>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginEditor::Private
{
public:

    Private() = default;

    QList<DPluginAction*> actions;
};

DPluginEditor::DPluginEditor(QObject* const parent)
    : DPlugin(parent),
      d      (new Private)
{
}

DPluginEditor::~DPluginEditor()
{
    delete d;
}

DInfoInterface* DPluginEditor::infoIface(QObject* const ac) const
{
    DPluginAction* const pac = dynamic_cast<DPluginAction*>(ac);

    if (pac)
    {
        DXmlGuiWindow* const gui = dynamic_cast<DXmlGuiWindow*>(pac->parent());

        if (gui)
        {
            return gui->infoIface(pac);
        }

        DInfoInterface* const iface = dynamic_cast<DInfoInterface*>(pac->parent());

        if (iface)
        {
            return iface;
        }
    }

    return nullptr;
}

int DPluginEditor::count() const
{
    int count       = 0;
    const QObject* parent = nullptr;

    for (DPluginAction* const ac : std::as_const(d->actions))
    {
        if (ac)
        {
            // NOTE: we will return the count of actions registered with the same parents,
            //       as each parent registered the same list of actions through setup().

            if      (!count)
            {
                parent = ac->parent(),
                ++count;
            }
            else if (ac->parent() == parent)
            {
                ++count;
            }
        }
    }

    return count;
}

void DPluginEditor::setVisible(bool b)
{
    for (DPluginAction* const ac : std::as_const(d->actions))
    {
        if (ac)
        {
            ac->setVisible(b);
        }
    }
}

QList<DPluginAction*> DPluginEditor::actions(QObject* const parent) const
{
    QList<DPluginAction*> list;

    for (DPluginAction* const ac : std::as_const(d->actions))
    {
        if (ac && (ac->parent() == parent))
        {
            list << ac;
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);

    return list;
}

DPluginAction* DPluginEditor::findActionByName(const QString& name, QObject* const parent) const
{
    const auto acs = actions(parent);

    for (DPluginAction* const ac : acs)
    {
        if (ac && (ac->objectName() == name))
        {    // cppcheck-suppress useStlAlgorithm
            return ac;
        }
    }

    return nullptr;
}

void DPluginEditor::addAction(DPluginAction* const ac)
{
    ac->setProperty("DPluginIId",      iid());
    ac->setProperty("DPluginIfaceIId", ifaceIid());
    d->actions.append(ac);
}

QStringList DPluginEditor::categories() const
{
    QStringList list;

    for (const DPluginAction* const ac : std::as_const(d->actions))
    {
        QString cat = ac->actionCategoryToString();

        if (!list.contains(cat))
        {
            list << cat;
        }
    }

    list.sort();

    return list;
}

} // namespace Digikam

#include "moc_dplugineditor.cpp"
