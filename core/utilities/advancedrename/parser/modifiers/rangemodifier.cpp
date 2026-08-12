/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for displaying only a range of a token result
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rangemodifier.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QPointer>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "ui_rangemodifierdialogwidget.h"

namespace Digikam
{

RangeDialog::RangeDialog(Rule* const parent, QWidget* const widget)
    : RuleDialog(parent, widget),
      ui        (new Ui::RangeModifierDialogWidget())
{
    QWidget* const mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
    ui->startInput->setFocus();

    slotToTheEndChecked(true);

    connect(ui->toTheEndCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToTheEndChecked(bool)));
}

RangeDialog::~RangeDialog()
{
    delete ui;
}

void RangeDialog::slotToTheEndChecked(bool checked)
{
    ui->stopInput->setEnabled(!checked);
}

// --------------------------------------------------------

RangeModifier::RangeModifier(QWidget* const widget)
    : Modifier(i18n("Range..."),
               i18n("Add only a specific range of a renaming option"),
               QLatin1String("measure"))
{
    addToken(QLatin1String("{range:||from||,||to||}"),
             i18n("Extract a specific range (if '||to||' is omitted, go to the end of string)"));

    QRegularExpression reg(QLatin1String("\\{range(:(-?\\d+)(,((-1|\\d+))?)?)\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
    setParentWidget(widget);
}

void RangeModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<RangeDialog> dlg = new RangeDialog(this, getParentWidget());

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        int start = dlg->ui->startInput->value();
        int stop  = dlg->ui->stopInput->value();

        if (dlg->ui->toTheEndCheckBox->isChecked())
        {
            result = QString::fromUtf8("{range:%1,}").arg(start);
        }
        else
        {
            result = QString::fromUtf8("{range:%1,%2}").arg(start).arg(stop);
        }
    }

    delete dlg;

    Q_EMIT signalTokenTriggered(result);
}

QString RangeModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    bool ok                       = false;

    // if the start parameter can not be extracted or is a negative value, set it to 1

    int start = match.captured(2).simplified().toInt(&ok);

    if (!ok || start < 0)
    {
        start = 1;
    }

    // If no range is defined at all ({start}), set stop = start.
    // If the stop parameter is omitted ({start-}), set stop = -1 (end of string)

    ok = false;
    int stop;

    if (!match.captured(3).isEmpty())
    {
        ok   = true;
        stop = (match.captured(4).isEmpty()) ? -1
                                             : match.captured(5).simplified().toInt(&ok);
    }
    else
    {
        stop = start;
    }

    if (!ok)
    {
        stop = start;
    }

    // --------------------------------------------------------

    // replace the string according to the given range

    if (start > settings.str2Modify.size())
    {
        return QString();
    }

    if (stop > settings.str2Modify.size())
    {
        stop = -1;
    }

    --start;

    if (stop != -1)
    {
        --stop;
    }

    if ((start < 0) || (stop < -1))
    {
        return QString();
    }

    if (stop == -1)
    {
        stop = settings.str2Modify.size() - 1;
    }

    QString result;

    for (int i = start ; i <= stop ; ++i)
    {
        result.append(settings.str2Modify.at(i));
    }

    return result;
}

} // namespace Digikam

#include "moc_rangemodifier.cpp"
