/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "replacemodifier.h"

// Qt includes

#include <QCheckBox>
#include <QPointer>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "ui_replacemodifierdialogwidget.h"

namespace Digikam
{

ReplaceDialog::ReplaceDialog(Rule* const parent, QWidget* const widget)
    : RuleDialog(parent, widget),
      ui        (new Ui::ReplaceModifierDialogWidget())
{
    QWidget* const mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
    ui->source->setFocus();
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

// --------------------------------------------------------

ReplaceModifier::ReplaceModifier(QWidget* const widget)
    : Modifier(i18nc("Replace text", "Replace..."), i18n("Replace text in a renaming option"),
               QLatin1String("document-edit"))
{
    addToken(QLatin1String("{replace:\"||old||\", \"||new||\",||options||}"),
             i18n("Replace text (||options||: ||r|| = regular expression, ||i|| = ignore case)"));

    QRegularExpression reg(QLatin1String("\\{replace(:\"(.*)\",\"(.*)\"(,(r|ri|ir|i))?)\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
    setParentWidget(widget);
}

void ReplaceModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<ReplaceDialog> dlg = new ReplaceDialog(this, getParentWidget());

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        QString oldStr = dlg->ui->source->text();
        QString newStr = dlg->ui->destination->text();

        if (!oldStr.isEmpty())
        {
            QString options;

            if (dlg->ui->isRegExp->isChecked())
            {
                options.append(QLatin1Char('r'));
            }

            if (!dlg->ui->caseSensitive->isChecked())
            {
                options.append(QLatin1Char('i'));
            }

            if (!options.isEmpty())
            {
                options.prepend(QLatin1Char(','));
            }

            result = QString::fromUtf8("{replace:\"%1\",\"%2\"%3}").arg(oldStr).arg(newStr).arg(options);
        }
    }

    delete dlg;

    Q_EMIT signalTokenTriggered(result);
}

QString ReplaceModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString original               = match.captured(2);
    QString replacement            = match.captured(3);
    QString result                 = settings.str2Modify;
    QString options                = match.captured(5);
    Qt::CaseSensitivity caseType = (!options.isEmpty() && options.contains(QLatin1Char('i'))) ? Qt::CaseInsensitive
                                                                                              : Qt::CaseSensitive;

    QRegularExpression ro(original);

    if (caseType == Qt::CaseInsensitive)
    {
        ro.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    if (!options.isEmpty() && options.contains(QLatin1Char('r')))
    {
        result.replace(ro, replacement);
    }
    else
    {
        result.replace(original, replacement, caseType);
    }

    return result;
}

} // namespace Digikam

#include "moc_replacemodifier.cpp"
