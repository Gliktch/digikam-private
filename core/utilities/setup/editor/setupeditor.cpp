/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup Image Editor page
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupeditor.h"

// Qt includes

#include <QTabWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "setupeditoriface.h"
#include "setupiofiles.h"
#include "setupversioning.h"
#include "setupraw.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupEditor::Private
{
public:

    Private() = default;

public:

    QTabWidget*       tab        = nullptr;

    SetupEditorIface* iface      = nullptr;
    SetupIOFiles*     iofiles    = nullptr;
    SetupVersioning*  versioning = nullptr;
    SetupRaw*         raw        = nullptr;
};

SetupEditor::SetupEditor(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab        = new QTabWidget;

    // --------------------------------------------------------

    d->iface      = new SetupEditorIface(d->tab);
    d->versioning = new SetupVersioning(d->tab);
    d->iofiles    = new SetupIOFiles(d->tab);

    // --------------------------------------------------------

    d->tab->insertTab(EditorWindow, d->iface,      i18nc("@title:tab", "Editor Window"));
    d->tab->insertTab(Versioning,   d->versioning, i18nc("@title:tab", "Versioning"));
    d->tab->insertTab(SaveSettings, d->iofiles,    i18nc("@title:tab", "Save Settings"));

    d->raw        = new SetupRaw(d->tab);

    // --------------------------------------------------------

    setWidget(d->tab);
    setWidgetResizable(true);
    readSettings();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::setActiveTab(EditorTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupEditor::EditorTab SetupEditor::activeTab() const
{
    return (EditorTab)d->tab->currentIndex();
}

void SetupEditor::applySettings()
{
    d->iface->applySettings();
    d->versioning->applySettings();
    d->iofiles->applySettings();
    d->raw->applySettings();
}

void SetupEditor::readSettings()
{
    // Nothing todo. All is already processed in widget constructors
}

} // namespace Digikam

#include "moc_setupeditor.cpp"
