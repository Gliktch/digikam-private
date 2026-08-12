/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : action container for external plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginaction.h"

// Qt includes

#include <QVariant>
#include <QCollator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DPluginAction::DPluginAction(QObject* const parent)
    : QAction(parent)
{
}

DPluginAction::ActionType DPluginAction::actionType() const
{
    switch (actionCategory())
    {
        case GenericExport:
        case GenericImport:
        case GenericTool:
        case GenericMetadata:
        case GenericView:
        {
            return Generic;
        }

        case EditorFile:
        case EditorColors:
        case EditorEnhance:
        case EditorTransform:
        case EditorDecorate:
        case EditorFilters:
        {
            return Editor;
        }

        default:
        {
            break;
        }
    }

    return InvalidType;
}

void DPluginAction::setActionCategory(ActionCategory cat)
{
    setProperty("DPluginActionCategory", (int)cat);
}

DPluginAction::ActionCategory DPluginAction::actionCategory() const
{
    bool b = false;
    int v  = property("DPluginActionCategory").toInt(&b);

    if (b)
    {
        return (ActionCategory)v;
    }

    return InvalidCat;
}

QString DPluginAction::actionCategoryToString() const
{
    switch (actionCategory())
    {
        case GenericExport:
        {
            return i18nc("@action: dplugin", "Export");
        }

        case GenericImport:
        {
            return i18nc("@action: dplugin", "Import");
        }

        case GenericTool:
        {
            return i18nc("@action: dplugin", "Tool");
        }

        case GenericMetadata:
        {
            return i18nc("@action: dplugin", "Metadata");
        }

        case GenericView:
        {
            return i18nc("@action: dplugin", "View");
        }

        // ---

        case EditorFile:
        {
            return i18nc("@action: dplugin", "File");
        }

        case EditorColors:
        {
            return i18nc("@action: dplugin", "Color");
        }

        case EditorEnhance:
        {
            return i18nc("@action: dplugin", "Enhance");
        }

        case EditorTransform:
        {
            return i18nc("@action: dplugin", "Transform");
        }

        case EditorDecorate:
        {
            return i18nc("@action: dplugin", "Decorate");
        }

        case EditorFilters:
        {
            return i18nc("@action: dplugin", "Effects");
        }

        // ---

        default:
        {
            break;
        }
    }

    return i18nc("@action: dplugin", "Invalid");
}

QString DPluginAction::xmlSection() const
{
    return QString::fromLatin1("<Action name=\"%1\" />\n").arg(objectName());
}

QString DPluginAction::toString() const
{
    return QString::fromUtf8("%1: \"%2\" - %3").arg(objectName())
                                               .arg(text())
                                               .arg(actionCategoryToString());
}

QString DPluginAction::pluginId() const
{
    return property("DPluginId").toString();
}

bool DPluginAction::pluginActionLessThan(DPluginAction* const a, DPluginAction* const b)
{
    QCollator collator;
    collator.setNumericMode(true);
    collator.setIgnorePunctuation(false);
    collator.setCaseSensitivity(Qt::CaseSensitive);

    return (collator.compare(a->text(), b->text()) < 0);
}

} // namespace Digikam

#include "moc_dpluginaction.cpp"
