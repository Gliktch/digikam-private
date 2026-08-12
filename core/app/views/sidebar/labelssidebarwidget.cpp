/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Labels sidebar widgets
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "labelssidebarwidget.h"

// Qt includes

#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QPushButton>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "dexpanderbox.h"
#include "imagequalitywidget.h"
#include "imagequalitysorter.h"

namespace Digikam
{

class Q_DECL_HIDDEN LabelsSideBarWidget::Private
{
public:

    Private() = default;

public:

    QPushButton*        helpButton   = nullptr;
    ImageQualityWidget* settingsWdg  = nullptr;
    QPushButton*        rescanButton = nullptr;

    DLabelExpander*     scanExpander = nullptr;

    LabelsTreeView*     labelsTree   = nullptr;
};

LabelsSideBarWidget::LabelsSideBarWidget(QWidget* const parent)
    : SidebarWidget(parent),
      d            (new Private)
{
    setObjectName(QLatin1String("Labels Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F3));

    const int spacing              = layoutSpacing();
    QVBoxLayout* const layout      = new QVBoxLayout(this);

    d->labelsTree                  = new LabelsTreeView(this);
    d->labelsTree->setConfigGroup(getConfigGroup());


    d->scanExpander                = new DLabelExpander(this);
    d->scanExpander->setText(i18n("Image Quality Scan"));
    d->scanExpander->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->scanExpander->setObjectName(QLatin1String("ImageQualityScanWidgetExpanded"));

    QWidget* const imgqsortWdg     = new QWidget(d->scanExpander);
    QVBoxLayout* const imgqsortLay = new QVBoxLayout(imgqsortWdg);

    d->settingsWdg    = new ImageQualityWidget(ImageQualityWidget::SettingsDisplayMode::Normal, imgqsortWdg);

    DHBox* const hbox = new DHBox(d->scanExpander);
    d->rescanButton   = new QPushButton(hbox);
    d->rescanButton->setText(i18n("Image Quality Scan"));
    d->rescanButton->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    d->rescanButton->setWhatsThis(i18nc("@info", "Use this button to scan the selected albums for image quality parsing"));

    d->helpButton     = new QPushButton(hbox);
    d->helpButton->setToolTip(i18nc("@info", "Help"));
    d->helpButton->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
    hbox->setStretchFactor(d->rescanButton, 10);

    QFontMetrics fmt  = d->rescanButton->fontMetrics();
    d->helpButton->setIconSize(QSize(fmt.height(), fmt.height()));

    connect(d->helpButton, &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("left_sidebar"),
                                    QLatin1String("labels_view"));
        }
    );

    imgqsortLay->addWidget(d->settingsWdg);
    imgqsortLay->addWidget(hbox);
    imgqsortLay->setContentsMargins(0, spacing, 0, 0);

    d->scanExpander->setLineVisible(true);
    d->scanExpander->setWidget(imgqsortWdg);
    d->scanExpander->setExpandByDefault(true);
    d->scanExpander->layout()->setContentsMargins(0, 0, 0, spacing);

    layout->addWidget(d->labelsTree, 10);
    layout->addWidget(d->scanExpander);
    layout->setContentsMargins(0, spacing, spacing, 0);

    connect(d->rescanButton, SIGNAL(pressed()),
            this, SLOT(slotScanForImageQuality()));
}

LabelsSideBarWidget::~LabelsSideBarWidget()
{
    delete d;
}

LabelsTreeView *LabelsSideBarWidget::labelsTree()
{
    return d->labelsTree;
}

void LabelsSideBarWidget::setActive(bool active)
{
    if (active)
    {
        d->labelsTree->setCurrentAlbum();
    }
}

void LabelsSideBarWidget::applySettings()
{
}

void LabelsSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    Q_UNUSED(album);
}

void LabelsSideBarWidget::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->scanExpander->setExpanded(group.readEntry(d->scanExpander->objectName(),
                                                 d->scanExpander->isExpandByDefault()));

    d->settingsWdg->loadState();
    d->labelsTree->loadState();
}

void LabelsSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(d->scanExpander->objectName(),
                     d->scanExpander->isExpanded());

    d->settingsWdg->saveState();
    d->labelsTree->saveState();
}

const QIcon LabelsSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("folder-favorites"));
}

const QString LabelsSideBarWidget::getCaption()
{
    return i18n("Labels");
}

QHash<LabelsTreeView::Labels, QList<int> > LabelsSideBarWidget::selectedLabels()
{
    return d->labelsTree->selectedLabels();
}

void LabelsSideBarWidget::slotScanForImageQuality()
{
    ImageQualitySettings imgqsortSettings = d->settingsWdg->settings();
    doImageQualityScan(imgqsortSettings);
}

void LabelsSideBarWidget::slotScanComplete()
{
    d->settingsWdg->setEnabled(true);
    d->rescanButton->setEnabled(true);
}

void LabelsSideBarWidget::doImageQualityScan(const ImageQualitySettings& imgqsortSettings)
{
    ImageQualitySorter* const imgqsortDetector = new ImageQualitySorter(imgqsortSettings);
    imgqsortDetector->setUseMultiCoreCPU(imgqsortSettings.useFullCpu);
    imgqsortDetector->start();

    connect(imgqsortDetector, SIGNAL(signalComplete()),
            this, SLOT(slotScanComplete()));

    connect(imgqsortDetector, SIGNAL(signalCanceled()),
            this, SLOT(slotScanComplete()));

    connect(imgqsortDetector, SIGNAL(signalScanNotification(QString,int)),
            this, SIGNAL(signalNotificationError(QString,int)));

    d->settingsWdg->setEnabled(false);
    d->rescanButton->setEnabled(false);
}

} // namespace Digikam

#include "moc_labelssidebarwidget.cpp"
