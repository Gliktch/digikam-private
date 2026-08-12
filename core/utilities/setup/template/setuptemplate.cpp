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

SetupTemplate::SetupTemplate(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing    = layoutSpacing();

    d->listView          = new TemplateList(panel);
    d->listView->setFixedHeight(100);

    // --------------------------------------------------------

    QLabel* const label0 = new QLabel(i18n("Template Title:"), panel);
    d->titleEdit         = new DTextEdit(panel);
    d->titleEdit->setLinesVisible(1);
    d->titleEdit->setPlaceholderText(i18n("Enter the metadata template title here."));
    d->titleEdit->setWhatsThis(i18n("<p>Enter the metadata template title here. This title will be "
                                    "used to identify a template in your collection.</p>"));
    label0->setBuddy(d->titleEdit);

    d->mergeCBox         = new QCheckBox(i18n("Merge Template"), this);

    // --------------------------------------------------------

    d->tview             = new TemplatePanel(panel);

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: These information are used to set "
                                   "<b><a href='https://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a></b> "
                                   "and <b><a href='https://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> tag contents. "
                                   "There is no limitation with XMP, but note that IPTC text tag sizes are limited. "
                                   "Use contextual help for details.</b>"), panel);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // -------------------------------------------------------------

    d->addButton = new QPushButton(panel);
    d->delButton = new QPushButton(panel);
    d->repButton = new QPushButton(panel);

    d->addButton->setText(i18n("&Add..."));
    d->addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delButton->setText(i18n("&Remove"));
    d->delButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    d->repButton->setText(i18n("&Replace..."));
    d->repButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delButton->setEnabled(false);
    d->repButton->setEnabled(false);

    // -------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout;
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    grid->setAlignment(Qt::AlignTop);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(4, 10);
    grid->addWidget(d->listView,  0, 0, 4, 2);
    grid->addWidget(d->addButton, 0, 2, 1, 1);
    grid->addWidget(d->delButton, 1, 2, 1, 1);
    grid->addWidget(d->repButton, 2, 2, 1, 1);
    grid->addWidget(label0,       4, 0, 1, 1);
    grid->addWidget(d->titleEdit, 4, 1, 1, 1);
    grid->addWidget(d->mergeCBox, 4, 2, 1, 1);
    grid->addWidget(d->tview,     5, 0, 1, 3);
    grid->addWidget(note,         6, 0, 1, 3);
    panel->setLayout(grid);

    panel->setTabOrder(d->listView,  d->addButton);
    panel->setTabOrder(d->addButton, d->delButton);
    panel->setTabOrder(d->delButton, d->repButton);
    panel->setTabOrder(d->repButton, d->titleEdit);
    panel->setTabOrder(d->titleEdit, d->tview);

    // --------------------------------------------------------

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddTemplate()));

    connect(d->delButton, SIGNAL(clicked()),
            this, SLOT(slotDelTemplate()));

    connect(d->repButton, SIGNAL(clicked()),
            this, SLOT(slotRepTemplate()));

    // --------------------------------------------------------

    readSettings();
    d->titleEdit->setFocus();

    d->listView->sortItems(0, Qt::AscendingOrder);
    d->listView->setSortingEnabled(true);
}

SetupTemplate::~SetupTemplate()
{
    delete d;
}

void SetupTemplate::setTemplate(const Template& t)
{
    if (!t.isNull())
    {
        TemplateListItem* const item = d->listView->find(t.templateTitle());
        d->listView->setCurrentItem(item);

        return;
    }

    populateTemplate(t);
}

void SetupTemplate::populateTemplate(const Template& t)
{
    d->tview->setTemplate(t);
    d->titleEdit->setText(t.templateTitle());
    d->mergeCBox->setChecked(t.templateMerge());
    d->titleEdit->setFocus();
}

} // namespace Digikam

#include "moc_setuptemplate.cpp"
