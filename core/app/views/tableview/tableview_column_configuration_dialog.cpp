/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Column configuration dialog
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_column_configuration_dialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnailloadthread.h"
#include "digikam_debug.h"

namespace Digikam
{

class ItemAlbumModel;
class ItemFilterModel;

class Q_DECL_HIDDEN TableViewConfigurationDialog::Private
{
public:

    Private() = default;

    int                                 columnIndex                 = 0;

    QDialogButtonBox*                   buttons                     = nullptr;

    TableViewColumn*                    columnObject                = nullptr;
    TableViewColumnConfigurationWidget* columnConfigurationWidget   = nullptr;
};

TableViewConfigurationDialog::TableViewConfigurationDialog(TableViewShared* const sharedObject,
                                                           const int columnIndex,
                                                           QWidget* const parentWidget)
    : QDialog(parentWidget),
      d      (new Private),
      s      (sharedObject)
{
    d->columnIndex               = columnIndex;
    d->columnObject              = s->tableViewModel->getColumnObject(d->columnIndex);
    d->columnConfigurationWidget = d->columnObject->getConfigurationWidget(this);

    if (d->columnObject)
    {
        setWindowTitle(i18n("Configure column \"%1\"", d->columnObject->getTitle()));
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Column object from TableView is null";
    }

    d->buttons             = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->columnConfigurationWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

TableViewConfigurationDialog::~TableViewConfigurationDialog()
{
}

TableViewColumnConfiguration TableViewConfigurationDialog::getNewConfiguration() const
{
    if (d->columnConfigurationWidget)
    {
        return d->columnConfigurationWidget->getNewConfiguration();
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Configuration widget from TableView is null";

        return TableViewColumnConfiguration();
    }
}

} // namespace Digikam

#include "moc_tableview_column_configuration_dialog.cpp"
