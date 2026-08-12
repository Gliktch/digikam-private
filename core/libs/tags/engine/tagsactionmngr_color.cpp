/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager - Color Labels
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsactionmngr_p.h"

namespace Digikam
{

QString TagsActionMngr::colorShortcutPrefix() const
{
    return d->colorShortcutPrefix;
}

bool TagsActionMngr::createColorLabelActionShortcut(KActionCollection* const ac, int colorId)
{
    if (ac)
    {
        QAction* const action = ac->addAction(QString::fromUtf8("%1-%2")
                                                  .arg(d->colorShortcutPrefix).arg(colorId));

        action->setText(i18n("Assign Color Label \"%1\"", d->colorLabelNames.value(colorId)));
        ac->setDefaultShortcut(action, QKeySequence(QString::fromUtf8("ALT+CTRL+%1").arg(colorId)));
        action->setIcon(ColorLabelWidget::buildIcon((ColorLabel)colorId, 32));
        action->setData(colorId);
        action->setToolTip(d->colorLabelNames.value(colorId));

        connect(action, SIGNAL(triggered()),
                this, SLOT(slotAssignFromShortcut()));

        return true;
    }

    return false;
}

void TagsActionMngr::loadColorNames()
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup group       = config->group(d->configColorNamesGroup);

    for (int i = FirstColorLabel ; i <= LastColorLabel ; i ++)
    {
        QString color = colorSet().value(i);
        QString name  = group.readEntry(d->configColorNameEntry + color, ColorLabelWidget::labelColorName((ColorLabel)i));
        d->colorLabelNames.insert(i, name);
    }
}

void TagsActionMngr::saveColorNames()
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup group       = config->group(d->configColorNamesGroup);

    for (int i = FirstColorLabel ; i <= LastColorLabel ; i ++)
    {
        QString color = colorSet().value(i);
        group.writeEntry(d->configColorNameEntry + color, d->colorLabelNames.value(i));
    }
}

QMap<int, QString> TagsActionMngr::colorLabelNames() const
{
    return d->colorLabelNames;
}

void TagsActionMngr::slotColorNameChanged(int label, const QString& name)
{
    d->colorLabelNames.insert(label, name);

    for (KActionCollection* const col : std::as_const(d->actionCollectionList))
    {
        const auto acs = col->actions();

        for (QAction* const ac : acs)
        {
            if ((ac->objectName().startsWith(d->colorShortcutPrefix)) && (ac->data() == label))
            {
                ac->setText(i18n("Assign Color Label \"%1\"", d->colorLabelNames.value(label)));
                ac->setToolTip(d->colorLabelNames.value(label));
            }
        }
    }

    Q_EMIT signalColorLabelNamesUpdated();
}

QStringList TagsActionMngr::colorSet()
{
   return (
           QStringList() << QLatin1String("nocolor")
                         << QLatin1String("red")      << QLatin1String("orange")
                         << QLatin1String("yellow")   << QLatin1String("darkgreen")
                         << QLatin1String("darkblue") << QLatin1String("magenta")
                         << QLatin1String("darkgray") << QLatin1String("black")
                         << QLatin1String("white")
          );
}

} // namespace Digikam
