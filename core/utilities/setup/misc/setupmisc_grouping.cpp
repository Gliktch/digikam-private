/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmisc_p.h"

namespace Digikam
{

void SetupMisc::setupGrouping()
{
    // -- Grouping Options -------------------------------------

    QWidget* const groupingPanel = new QWidget(d->tab);
    QGridLayout* const grid      = new QGridLayout(groupingPanel);

    QLabel* const description    = new QLabel(i18n("Perform the following operations on all group members:"), groupingPanel);
    description->setToolTip(i18n("When images are grouped the following operations<br>"
                                 "are performed only on the displayed item (No)<br>"
                                 "or on all the hidden items in the group as well (Yes).<br>"
                                 "If Ask is selected, there will be a prompt every<br>"
                                 "time this operation is executed."));

    QLabel* const noLabel        = new QLabel(i18nc("@label: grouped image ops", "No"),  groupingPanel);
    QLabel* const yesLabel       = new QLabel(i18nc("@label: grouped image ops", "Yes"), groupingPanel);
    QLabel* const askLabel       = new QLabel(i18nc("@label: grouped image ops", "Ask"), groupingPanel);

    QHash<int, QLabel*> labels;

    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        labels.insert(i, new QLabel(ApplicationSettings::operationTypeTitle(
                                    (OperationType)i), groupingPanel));
        QString explanation = ApplicationSettings::operationTypeExplanation(
                              (OperationType)i);

        if (!explanation.isEmpty())
        {
            labels.value(i)->setToolTip(explanation);
        }

        d->groupingButtons.insert(i, new QButtonGroup(groupingPanel));
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 0);
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 1);
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 2);
    }

    grid->addWidget(description, 0, 0, 1, 4);
    grid->addWidget(noLabel,     1, 1, 1, 1);
    grid->addWidget(yesLabel,    1, 2, 1, 1);
    grid->addWidget(askLabel,    1, 3, 1, 1);

    for (int i = 0 ; i != UnspecifiedOps ; ++i)
    {
        grid->addWidget(labels.value(i),                        i+2, 0, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(0), i+2, 1, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(1), i+2, 2, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(2), i+2, 3, 1, 1);
    }

    grid->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid->setSpacing(d->spacing);
    grid->setColumnStretch(0, 10);
    grid->setColumnMinimumWidth(1, 30);
    grid->setColumnMinimumWidth(2, 30);
    grid->setColumnMinimumWidth(3, 30);
    grid->setRowStretch(20, 10);

    d->tab->insertTab(Grouping, groupingPanel, i18nc("@title:tab", "Grouping"));
}

} // namespace Digikam
