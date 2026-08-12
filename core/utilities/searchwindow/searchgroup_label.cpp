/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchgroup_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN SearchGroupLabel::Private
{
public:

    Private() = default;

    bool                        extended        = false;
    SearchXml::Operator         groupOp         = SearchXml::And;
    SearchXml::Operator         fieldOp         = SearchXml::And;
    QGridLayout*                layout          = nullptr;
/*
    QComboBox*                  groupOpBox      = nullptr;
*/
    DClickLabel*                groupOpLabel    = nullptr;
    QRadioButton*               allBox          = nullptr;
    QRadioButton*               anyBox          = nullptr;
    QRadioButton*               noneBox         = nullptr;
    QRadioButton*               oneNotBox       = nullptr;
    DClickLabel*                optionsLabel    = nullptr;
    DClickLabel*                removeLabel     = nullptr;
    QStackedLayout*             stackedLayout   = nullptr;
    SearchViewThemedPartsCache* themeCache      = nullptr;
};

SearchGroupLabel::SearchGroupLabel(SearchViewThemedPartsCache* const cache, SearchGroup::Type type, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->themeCache = cache;
    d->layout                    = new QGridLayout;

    // leave styling to style sheet (by object name)

    QLabel* const mainLabel      = new QLabel(i18n("Find Items"));
    mainLabel->setObjectName(QLatin1String("SearchGroupLabel_MainLabel"));

    // Use radio button with a separate label to fix styling problem, see bug 195809

    d->allBox                    = new QRadioButton;
    QLabel* const allBoxLabel    = new QLabel(i18n("Meet All of the following conditions"));
    allBoxLabel->setObjectName(QLatin1String("SearchGroupLabel_CheckBox"));

    d->anyBox                    = new QRadioButton;
    QLabel* const anyBoxLabel    = new QLabel(i18n("Meet Any of the following conditions"));
    anyBoxLabel->setObjectName(QLatin1String("SearchGroupLabel_CheckBox"));

    d->noneBox = new QRadioButton;
    QLabel* const noneBoxLabel   = new QLabel(i18n("None of these conditions are met"));
    noneBoxLabel->setObjectName(QLatin1String("SearchGroupLabel_CheckBox"));

    d->oneNotBox                 = new QRadioButton;
    QLabel* const oneNotBoxLabel = new QLabel(i18n("At least one of these conditions is not met"));
    oneNotBoxLabel->setObjectName(QLatin1String("SearchGroupLabel_CheckBox"));

    connect(d->allBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->anyBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->noneBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    connect(d->oneNotBox, SIGNAL(toggled(bool)),
            this, SLOT(boxesToggled()));

    if (type == SearchGroup::FirstGroup)
    {
        QLabel* const logo              = new QLabel;
        logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

        d->optionsLabel                 = new DClickLabel;
        d->optionsLabel->setObjectName(QLatin1String("SearchGroupLabel_OptionsLabel"));

        connect(d->optionsLabel, SIGNAL(activated()),
                this, SLOT(toggleShowOptions()));

        QWidget* const simpleHeader     = new QWidget;
        QVBoxLayout* const headerLayout = new QVBoxLayout;
        QLabel* const simpleLabel1      = new QLabel;
        simpleLabel1->setText(i18n("<qt><p>Search your collection<br>for Items meeting the following conditions</p></qt>"));
        simpleLabel1->setObjectName(QLatin1String("SearchGroupLabel_SimpleLabel"));
        headerLayout->addStretch(3);
        headerLayout->addWidget(simpleLabel1);
        headerLayout->addStretch(1);
        headerLayout->setContentsMargins(QMargins());
        simpleHeader->setLayout(headerLayout);

        QWidget* const optionsBox        = new QWidget;
        QGridLayout* const optionsLayout = new QGridLayout;
        optionsLayout->addLayout(new RadioButtonHBox(d->allBox,    allBoxLabel,    layoutDirection()), 0, 0);
        optionsLayout->addLayout(new RadioButtonHBox(d->anyBox,    anyBoxLabel,    layoutDirection()), 1, 0);
        optionsLayout->addLayout(new RadioButtonHBox(d->noneBox,   noneBoxLabel,   layoutDirection()), 0, 1);
        optionsLayout->addLayout(new RadioButtonHBox(d->oneNotBox, oneNotBoxLabel, layoutDirection()), 1, 1);
        optionsLayout->setContentsMargins(QMargins());
        optionsBox->setLayout(optionsLayout);

        d->stackedLayout = new QStackedLayout;
        d->stackedLayout->addWidget(simpleHeader);
        d->stackedLayout->addWidget(optionsBox);
        d->stackedLayout->setContentsMargins(QMargins());

        d->layout->addWidget(mainLabel,        0, 0, 1, 1);
        d->layout->addLayout(d->stackedLayout, 1, 0, 1, 1);
        d->layout->addWidget(d->optionsLabel,  1, 1, 1, 1, Qt::AlignRight | Qt::AlignBottom);
        d->layout->addWidget(logo,             0, 2, 2, 1, Qt::AlignTop);
        d->layout->setColumnStretch(1, 10);

        setExtended(false);
    }
    else
    {
        d->groupOpLabel = new DClickLabel;
        d->groupOpLabel->setObjectName(QLatin1String("SearchGroupLabel_GroupOpLabel"));

        connect(d->groupOpLabel, SIGNAL(activated()),
                this, SLOT(toggleGroupOperator()));

        d->removeLabel  = new DClickLabel(i18n("Remove Group"));
        d->removeLabel->setObjectName(QLatin1String("SearchGroupLabel_RemoveLabel"));

        connect(d->removeLabel, SIGNAL(activated()),
                this, SIGNAL(removeClicked()));

        d->layout->addWidget(d->groupOpLabel, 0, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->allBox, allBoxLabel, layoutDirection()),       1, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->anyBox, anyBoxLabel, layoutDirection()),       2, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->noneBox, noneBoxLabel, layoutDirection()),     3, 0, 1, 1);
        d->layout->addLayout(new RadioButtonHBox(d->oneNotBox, oneNotBoxLabel, layoutDirection()), 4, 0, 1, 1);
        d->layout->addWidget(d->removeLabel,  0, 2, 1, 1);
        d->layout->setColumnStretch(1, 10);
    }

    setLayout(d->layout);

    // Default values

    setGroupOperator(SearchXml::standardGroupOperator());
    setDefaultFieldOperator(SearchXml::standardFieldOperator());
}

SearchGroupLabel::~SearchGroupLabel()
{
    delete d;
}

void SearchGroupLabel::setExtended(bool extended)
{
    d->extended = extended;

    if (!d->stackedLayout)
    {
        return;
    }

    if (d->extended)
    {
        d->stackedLayout->setCurrentIndex(1);
        d->allBox->setVisible(true);
        d->anyBox->setVisible(true);
        d->noneBox->setVisible(true);
        d->oneNotBox->setVisible(true);
        d->optionsLabel->setText(i18n("Hide Options") + QLatin1String(" <<"));
    }
    else
    {
        d->stackedLayout->setCurrentIndex(0);

        // hide to reduce reserved space in stacked layout

        d->allBox->setVisible(false);
        d->anyBox->setVisible(false);
        d->noneBox->setVisible(false);
        d->oneNotBox->setVisible(false);
        d->optionsLabel->setText(i18n("Options") + QLatin1String(" >>"));
    }
}

void SearchGroupLabel::toggleShowOptions()
{
    setExtended(!d->extended);
}

void SearchGroupLabel::toggleGroupOperator()
{
    if      (d->groupOp == SearchXml::And)
    {
        d->groupOp = SearchXml::Or;
    }
    else if (d->groupOp == SearchXml::Or)
    {
        d->groupOp = SearchXml::And;
    }
    else if (d->groupOp == SearchXml::AndNot)
    {
        d->groupOp = SearchXml::OrNot;
    }
    else if (d->groupOp == SearchXml::OrNot)
    {
        d->groupOp = SearchXml::AndNot;
    }

    updateGroupLabel();
}

void SearchGroupLabel::boxesToggled()
{
    // set field op

    if (d->allBox->isChecked() || d->oneNotBox->isChecked())
    {
        d->fieldOp = SearchXml::And;
    }
    else
    {
        d->fieldOp = SearchXml::Or;
    }

    // negate group op

    if (d->allBox->isChecked() || d->anyBox->isChecked())
    {
        if      (d->groupOp == SearchXml::AndNot)
        {
            d->groupOp = SearchXml::And;
        }
        else if (d->groupOp == SearchXml::OrNot)
        {
            d->groupOp = SearchXml::Or;
        }
    }
    else
    {
        if      (d->groupOp == SearchXml::And)
        {
            d->groupOp = SearchXml::AndNot;
        }
        else if (d->groupOp == SearchXml::Or)
        {
            d->groupOp = SearchXml::OrNot;
        }
    }
}

void SearchGroupLabel::setGroupOperator(SearchXml::Operator op)
{
    d->groupOp = op;
    adjustOperatorOptions();
    updateGroupLabel();
}

void SearchGroupLabel::updateGroupLabel()
{
    if (d->groupOpLabel)
    {
        if ((d->groupOp == SearchXml::And) || (d->groupOp == SearchXml::AndNot))
        {
            d->groupOpLabel->setText(i18n("AND"));
        }
        else
        {
            d->groupOpLabel->setText(i18n("OR"));
        }
    }
}

void SearchGroupLabel::setDefaultFieldOperator(SearchXml::Operator op)
{
    d->fieldOp = op;
    adjustOperatorOptions();
}

void SearchGroupLabel::adjustOperatorOptions()
{
    // In the UI, the NOT is done at the level of the field operator,
    // but in fact we put a NOT in front of the whole group, so it is the group operator!

    // 1. allBox, All of these conditions are met: (A && B && C), Group And/Or, Field And
    // 2. anyBox, Any of these conditions are met: (A || B || C), Group And/Or, Field Or
    // 3. oneNotBox, At least one of these conditions is not met: !(A && B && C) = (!A || !B || !C),
    //    Group AndNot/OrNot, Field And
    // 4. noneBox, None of these conditions are met: !(A || B || C) = (!A && !B && !C),
    //    Group AndNot/OrNot, Field Or

    switch (d->groupOp)
    {
        case SearchXml::And:
        case SearchXml::Or:
        {
            if (d->fieldOp == SearchXml::And)
            {
                d->allBox->setChecked(true);
            }
            else
            {
                d->anyBox->setChecked(true);
            }

            break;
        }

        case SearchXml::AndNot:
        case SearchXml::OrNot:
        {
            if (d->fieldOp == SearchXml::And)
            {
                d->oneNotBox->setChecked(true);
            }
            else
            {
                d->noneBox->setChecked(true);
            }

            break;
        }
    }

    if (!d->allBox->isChecked())
    {
        setExtended(true);
    }
}

SearchXml::Operator SearchGroupLabel::groupOperator() const
{
    return d->groupOp;
}

SearchXml::Operator SearchGroupLabel::defaultFieldOperator() const
{
    if (d->anyBox->isChecked() || d->noneBox->isChecked())
    {
        return SearchXml::Or;
    }
    else
    {
        return SearchXml::And;
    }
}

void SearchGroupLabel::paintEvent(QPaintEvent*)
{
    // paint themed background

    QPainter p(this);
    p.drawPixmap(0, 0, d->themeCache->groupLabelPixmap(width(), height()));
}

} // namespace Digikam
