/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setuptemplate_p.h"

namespace Digikam
{

void SetupTemplate::slotSelectionChanged()
{
    const TemplateListItem* const item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());

    if (!item)
    {
        d->delButton->setEnabled(false);
        d->repButton->setEnabled(false);

        return;
    }

    d->delButton->setEnabled(true);
    d->repButton->setEnabled(true);
    populateTemplate(item->getTemplate());
}

void SetupTemplate::slotAddTemplate()
{
    QString title = d->titleEdit->text();
    bool merge    = d->mergeCBox->isChecked();

    if (title.isEmpty())
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("Cannot register new metadata template without title."));

        return;
    }

    if (d->listView->find(title))
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("A metadata template named '%1' already exists.", title));

        return;
    }

    d->tview->apply();

    Template t = d->tview->getTemplate();
    t.setTemplateTitle(title);
    t.setTemplateMerge(merge);
    TemplateListItem* const item = new TemplateListItem(d->listView, t);
    d->listView->setCurrentItem(item);
}

void SetupTemplate::slotDelTemplate()
{
    TemplateListItem* const item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    delete item;
}

void SetupTemplate::slotRepTemplate()
{
    QString title = d->titleEdit->text();
    bool merge    = d->mergeCBox->isChecked();

    if (title.isEmpty())
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("Cannot register new metadata template without title."));

        return;
    }

    TemplateListItem* const item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    d->tview->apply();

    Template t = d->tview->getTemplate();
    t.setTemplateTitle(title);
    t.setTemplateMerge(merge);
    item->setTemplate(t);
}

} // namespace Digikam
