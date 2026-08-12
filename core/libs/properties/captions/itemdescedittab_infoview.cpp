/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor - Information view.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemdescedittab_p.h"

namespace Digikam
{

void ItemDescEditTab::initInformationView()
{
    QScrollArea* const sv2    = new QScrollArea(d->tabWidget);
    sv2->setFrameStyle(QFrame::NoFrame);
    sv2->setWidgetResizable(true);

    QWidget* const infoArea   = new QWidget(sv2->viewport());
    QGridLayout* const grid2  = new QGridLayout(infoArea);
    sv2->setWidget(infoArea);

    d->templateSelector       = new TemplateSelector(infoArea);
    d->templateViewer         = new TemplateViewer(infoArea);
    d->templateViewer->setObjectName(QLatin1String("ItemDescEditTab Expander"));

    grid2->addWidget(d->templateSelector, 0, 0, 1, 2);
    grid2->addWidget(d->templateViewer,   1, 0, 1, 2);
    grid2->setRowStretch(1, 10);
    grid2->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid2->setSpacing(d->spacing);

    d->tabWidget->insertTab(INFOS, sv2, i18nc("@title", "Information"));
}

void ItemDescEditTab::slotTemplateSelected()
{
    Template t = d->templateSelector->getTemplate();

    if (t.isNull())
    {
        return;
    }

    d->hub->setMetadataTemplate(t);
    d->templateViewer->setTemplate(t);
    d->setMetadataWidgetStatus(d->hub->templateStatus(), d->templateSelector);
    slotModified();
}

void ItemDescEditTab::updateTemplate()
{
    d->templateSelector->blockSignals(true);
    d->templateSelector->setTemplate(d->hub->metadataTemplate());
    d->templateViewer->setTemplate(d->hub->metadataTemplate());
    d->setMetadataWidgetStatus(d->hub->templateStatus(), d->templateSelector);
    d->templateSelector->blockSignals(false);
}

} // namespace Digikam
