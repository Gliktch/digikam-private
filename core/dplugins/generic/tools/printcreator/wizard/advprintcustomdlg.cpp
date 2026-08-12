/* ===============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-01
 * Description : Dialog to allow a custom page layout
 *
 * SPDX-FileCopyrightText: 2010-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================== */

#include "advprintcustomdlg.h"

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace DigikamGenericPrintCreatorPlugin
{

AdvPrintCustomLayoutDlg::AdvPrintCustomLayoutDlg(QWidget* const parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(m_doneButton, SIGNAL(clicked()),
            this, SLOT(accept()));

    m_photoGridCheck->setToolTip(i18n("Choose your grid size"));
    m_photoGridCheck->setWhatsThis(i18n("Choose your grid size"));
    m_gridRows->setToolTip(i18n("Number of rows"));
    m_gridRows->setWhatsThis(i18n("Insert number of rows"));
    m_gridColumns->setToolTip(i18n("Number of columns"));
    m_gridColumns->setWhatsThis(i18n("Insert number of columns"));

    m_fitAsManyCheck->setToolTip(i18n("Choose to have a custom photo size album"));
    m_fitAsManyCheck->setWhatsThis(i18n("Choose to have a custom photo size album"));
    m_photoHeight->setToolTip(i18n("Photo height"));
    m_photoHeight->setWhatsThis(i18n("Insert photo height"));
    m_photoWidth->setToolTip(i18n("Photo width"));
    m_photoWidth->setWhatsThis(i18n("Insert photo width"));

    m_autorotate->setToolTip(i18n("Rotate photo automatically on layout accordingly "
                                  "with camera orientation information"));
}

void AdvPrintCustomLayoutDlg::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("PrintCreator"));

    QSize gridSize     = group.readEntry(QLatin1String("Custom-gridSize"), QSize(3, 8));
    m_gridRows->setValue(gridSize.width());
    m_gridColumns->setValue(gridSize.height());

    QSizeF photoSize   = group.readEntry (QLatin1String("Custom-photoSize"), QSizeF(5.0, 4.0));
    m_photoHeight->setValue(photoSize.height());
    m_photoWidth->setValue(photoSize.width());

    int index          = group.readEntry(QLatin1String("Custom-photoUnits"), 0);
    m_photoUnits->setCurrentIndex(index);

    bool autorotate    = group.readEntry(QLatin1String("Custom-autorotate"), false);
    m_autorotate->setChecked(autorotate);

    int choice         = group.readEntry(QLatin1String("Custom-choice"), (int)PHOTO_GRID);

    if (choice == FIT_AS_MANY_AS_POSSIBLE)
    {
        m_fitAsManyCheck->setChecked(true);
    }
    else
    {
        m_photoGridCheck->setChecked(true);
    }
}

void AdvPrintCustomLayoutDlg::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("PrintCreator"));
    int choice              = PHOTO_GRID;

    if (m_fitAsManyCheck->isChecked())
    {
        choice = FIT_AS_MANY_AS_POSSIBLE;
    }

    group.writeEntry(QLatin1String("Custom-choice"),     choice);
    group.writeEntry(QLatin1String("Custom-gridSize"),   QSize(m_gridRows->value(),    m_gridColumns->value()));
    group.writeEntry(QLatin1String("Custom-photoSize"),  QSizeF(m_photoWidth->value(), m_photoHeight->value()));
    group.writeEntry(QLatin1String("Custom-photoUnits"), m_photoUnits->currentIndex());
    group.writeEntry(QLatin1String("Custom-autorotate"), m_autorotate->isChecked());
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintcustomdlg.cpp"
