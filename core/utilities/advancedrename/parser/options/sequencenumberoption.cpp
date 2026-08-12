/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sequencenumberoption.h"

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QGroupBox>
#include <QGridLayout>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "ui_sequencenumberoptiondialogwidget.h"

namespace Digikam
{

SequenceNumberDialog::SequenceNumberDialog(Rule* const parent, QWidget* const widget)
    : RuleDialog(parent, widget),
      ui(new Ui::SequenceNumberOptionDialogWidget)
{
    QWidget* const mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
}

SequenceNumberDialog::~SequenceNumberDialog()
{
    delete ui;
}

void SequenceNumberDialog::showEvent(QShowEvent* e)
{
    RuleDialog::showEvent(e);

    ui->digits->setFocus();
}

// --------------------------------------------------------

SequenceNumberOption::SequenceNumberOption(QWidget* const widget)
    : Option(i18nc("Sequence Number", "Number..."),
             i18n("Add a sequence number"),
             QLatin1String("accessories-calculator"))
{
    setParentWidget(widget);

    addToken(QLatin1String("#"),                                 i18n("Sequence number"));
    addToken(QLatin1String("#[||options||]"),                    i18n("Sequence number (||options||: "
                                                                      "||c|| = file counter aware, "
                                                                      "||e|| = extension aware, "
                                                                      "||f|| = folder aware, "
                                                                      "||r|| = random aware, "
                                                                      "||ce|| = counter and extension aware, "
                                                                      "||re|| = random and extension aware)"));
    addToken(QLatin1String("#[||options||,||start||]"),          i18n("Sequence number (custom start) "
                                                                      "||options||: ||e||, ||f||"));
    addToken(QLatin1String("#[||options||,||step||]"),           i18n("Sequence number (custom step) "
                                                                      "||options||: ||c||"));
    addToken(QLatin1String("#[||options||,||start||,||step||]"), i18n("Sequence number (custom start + step) "
                                                                      "||options||: ||e||, ||f||"));

    QRegularExpression reg(QLatin1String("(#+)(\\[(([cefr]?|ce?|re?)?,?)?((-?\\d+)(,(-?\\d+))?)?\\])?"));
    setRegExp(reg);
    setParentWidget(widget);
}

void SequenceNumberOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QPointer<SequenceNumberDialog> dlg = new SequenceNumberDialog(this, getParentWidget());

    QString result;

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        int digits          = dlg->ui->digits->value();
        int start           = dlg->ui->start->value();
        int step            = dlg->ui->step->value();
        bool extensionAware = (dlg->ui->extensionAware->isChecked()    ||
                               dlg->ui->randomAndExtAware->isChecked() ||
                               dlg->ui->counterAndExtAware->isChecked());
        bool counterAware   = (dlg->ui->counterAware->isChecked()      ||
                               dlg->ui->counterAndExtAware->isChecked());
        bool folderAware    = dlg->ui->folderAware->isChecked();
        bool randomAware    = (dlg->ui->randomAware->isChecked()      ||
                               dlg->ui->randomAndExtAware->isChecked());

        result = QString::fromUtf8("%1").arg(QLatin1String("#"), digits, QLatin1Char('#'));

        if ((start > 1) || (step > 1) || extensionAware || folderAware || counterAware || randomAware)
        {
            result.append(QLatin1Char('['));

            if (folderAware)
            {
                result.append(QLatin1Char('f'));
            }

            if (counterAware)
            {
                result.append(QLatin1Char('c'));
            }

            if (randomAware)
            {
                result.append(QLatin1Char('r'));
            }

            if (extensionAware)
            {
                result.append(QLatin1Char('e'));
            }

            if (
                !randomAware && !counterAware &&
                ((start  > 1) || ((start == 1) && (step > 1)))
               )
            {
                if (extensionAware || folderAware)
                {
                    result.append(QLatin1Char(','));
                }

                result.append(QString::number(start));
            }

            if (!randomAware && (step > 1))
            {
                result.append(QString::fromUtf8(",%1").arg(step));
            }

            result.append(QLatin1Char(']'));
        }
    }

    delete dlg;

    Q_EMIT signalTokenTriggered(result);
}

QString SequenceNumberOption::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString result;
    QString random;
    int slength         = 0;
    int start           = 0;
    int step            = 0;
    int number          = 0;
    int index           = 0;

    bool extensionAware = false;
    bool counterAware   = false;
    bool folderAware    = false;
    bool randomAware    = false;

    if (settings.manager)
    {
        extensionAware = !match.captured(3).isEmpty() && match.captured(3).contains(QLatin1Char('e'));
        counterAware   = !match.captured(3).isEmpty() && match.captured(3).contains(QLatin1Char('c'));
        folderAware    = !match.captured(3).isEmpty() && match.captured(3).contains(QLatin1Char('f'));
        randomAware    = !match.captured(3).isEmpty() && match.captured(3).contains(QLatin1Char('r'));

        index          = settings.manager->indexOfFile(settings.fileUrl.toLocalFile());

        if (extensionAware)
        {
            index = settings.manager->indexOfFileGroup(settings.fileUrl.toLocalFile());
        }

        if (counterAware)
        {
            if (extensionAware)
            {
                index = settings.manager->indexOfFileGroup(settings.fileUrl.toLocalFile());
            }
            else
            {
                index = settings.manager->indexOfFolder(settings.fileUrl.toLocalFile());
            }

            start = settings.manager->indexOfFileCounter(settings.fileUrl.toLocalFile());
        }

        if (folderAware)
        {
            index = settings.manager->indexOfFolder(settings.fileUrl.toLocalFile());
        }

        if (randomAware)
        {
            if (extensionAware)
            {
                index = settings.manager->indexOfFileGroup(settings.fileUrl.toLocalFile());
            }

            random = settings.manager->randomStringOfIndex(index - 1);
        }
    }

    // --------------------------------------------------------

    slength = match.captured(1).length();

    if (!counterAware)
    {
        start = match.captured(6).isEmpty() ? settings.startIndex : match.captured(6).toInt();
        step  = match.captured(8).isEmpty() ? 1 : match.captured(8).toInt();
    }
    else
    {
        step  = match.captured(5).isEmpty() ? 1 : match.captured(5).toInt();
    }

    if (step < 1)
    {
        step = 1;
    }

    if      (start < 1)
    {
        start = settings.startIndex;
    }
    else if (counterAware)
    {
        start += step;
    }

    if (randomAware && !random.isEmpty())
    {
         result = random.left(slength);
    }
    else
    {
        number  = start + ((index - 1) * step);
        result  = QString::fromUtf8("%1").arg(number, slength, 10, QLatin1Char('0'));
    }

    return result;
}

} // namespace Digikam

#include "moc_sequencenumberoption.cpp"
