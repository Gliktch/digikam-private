/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-11
 * Description : a modifier for setting a default value if option parsing failed
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "defaultvaluemodifier.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

DefaultValueDialog::DefaultValueDialog(Rule* parent, QWidget* const widget)
    : RuleDialog(parent, widget)
{
    const int spacing       = layoutSpacing();


    QString defaultValueStr = i18n("Default Value");

    QLabel* const srcLabel  = new QLabel(defaultValueStr + QLatin1Char(':'));
    valueInput              = new QLineEdit(this);
    valueInput->setToolTip(i18n("<p>Set a default value for empty strings.<br>"
                                "When applied to a renaming option, "
                                "an empty string will be replaced by the value you specify here.</p>"));

    QWidget* const mainWidget     = new QWidget(this);
    QGridLayout* const mainLayout = new QGridLayout(this);
    mainLayout->addWidget(srcLabel,   0, 0);
    mainLayout->addWidget(valueInput, 0, 1);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainLayout->setRowStretch(1, 10);
    mainWidget->setLayout(mainLayout);

    setSettingsWidget(mainWidget);

    valueInput->setFocus();
}

DefaultValueDialog::~DefaultValueDialog()
{
}

// --------------------------------------------------------

DefaultValueModifier::DefaultValueModifier(QWidget* const widget)
    : Modifier(i18nc("default value for empty strings", "Default Value..."),
               i18n("Set a default value for empty strings"),
               QLatin1String("edit-undo"))
{
    addToken(QLatin1String("{default:\"||value||\"}"), description());

    QRegularExpression reg(QLatin1String("\\{default:\"(.+)\"\\}"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
    setParentWidget(widget);
}

void DefaultValueModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<DefaultValueDialog> dlg = new DefaultValueDialog(this, getParentWidget());

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        QString valueStr = dlg->valueInput->text();

        if (!valueStr.isEmpty())
        {
            result = QString::fromUtf8("{default:\"%1\"}").arg(valueStr);
        }
    }

    delete dlg;

    Q_EMIT signalTokenTriggered(result);
}

QString DefaultValueModifier::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    if (!settings.str2Modify.isEmpty())
    {
        return settings.str2Modify;
    }

    QString defaultStr = match.captured(1).isEmpty() ? QString() : match.captured(1);

    return defaultStr;
}

} // namespace Digikam

#include "moc_defaultvaluemodifier.cpp"
