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

void SetupTemplate::applySettings()
{
    QString title = d->titleEdit->text();

    if (!title.isEmpty())
    {
        TemplateListItem* const item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());

        if (item)
        {
            d->tview->apply();

            Template t = d->tview->getTemplate();
            t.setTemplateTitle(title);
            t.setTemplateMerge(d->mergeCBox->isChecked());
            item->setTemplate(t);
        }
    }

    d->listView->applySettings();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));
    group.writeEntry(QLatin1String("Template Tab"), (int)(d->tview->currentIndex()));
    config->sync();
}

void SetupTemplate::readSettings()
{
    d->listView->readSettings();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));
    d->tview->setCurrentIndex((TemplatePanel::TemplateTab)group.readEntry(QLatin1String("Template Tab"), (int)TemplatePanel::RIGHTS));
}

} // namespace Digikam
