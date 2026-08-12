/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the time-line view.
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

#include "timelinesidebarwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QRadioButton>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "albumpointer.h"
#include "albummodificationhelper.h"
#include "albumselectiontreeview.h"
#include "applicationsettings.h"
#include "coredbsearchxml.h"
#include "datefolderview.h"
#include "dexpanderbox.h"
#include "editablesearchtreeview.h"
#include "timelinewidget.h"
#include "searchfolderview.h"
#include "searchtabheader.h"
#include "searchtextbardb.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN TimelineSideBarWidget::Private
{
public:

    Private() = default;

public:

    const QString             configHistogramTimeUnitEntry  = QLatin1String("Histogram TimeUnit");
    const QString             configHistogramScaleEntry     = QLatin1String("Histogram Scale");
    const QString             configCursorPositionEntry     = QLatin1String("Cursor Position");

    QButtonGroup*             scaleBG                       = nullptr;
    QLabel*                   cursorCountLabel              = nullptr;
    QScrollBar*               scrollBar                     = nullptr;
    QTimer*                   timer                         = nullptr;
    QToolButton*              resetButton                   = nullptr;
    QToolButton*              saveButton                    = nullptr;

    QComboBox*                timeUnitCB                    = nullptr;
    DTextEdit*                nameEdit                      = nullptr;
    DAdjustableLabel*         cursorDateLabel               = nullptr;

    SearchTextBarDb*          searchDateBar                 = nullptr;
    EditableSearchTreeView*   timeLineFolderView            = nullptr;
    TimeLineWidget*           timeLineWidget                = nullptr;

    SearchModificationHelper* searchModificationHelper      = nullptr;

    AlbumPointer<SAlbum>      currentTimelineSearch;
};

// --------------------------------------------------------

TimelineSideBarWidget::TimelineSideBarWidget(QWidget* const parent,
                                             SearchModel* const searchModel,
                                             SearchModificationHelper* const searchModificationHelper)
    : SidebarWidget(parent),
      d            (new Private)
{
    setObjectName(QLatin1String("TimeLine Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F5));

    d->searchModificationHelper = searchModificationHelper;
    d->timer                    = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing       = layoutSpacing();

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    QFrame* const panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout* const grid = new QGridLayout(panel);

    // ---------------------------------------------------------------

    QWidget* const hbox1    = new QWidget(panel);
    QHBoxLayout* const hlay = new QHBoxLayout(hbox1);

    QLabel* const label1    = new QLabel(i18n("Time Unit:"), hbox1);
    d->timeUnitCB           = new QComboBox(hbox1);
    d->timeUnitCB->addItem(i18n("Day"),   TimeLineWidget::Day);
    d->timeUnitCB->addItem(i18n("Week"),  TimeLineWidget::Week);
    d->timeUnitCB->addItem(i18n("Month"), TimeLineWidget::Month);
    d->timeUnitCB->addItem(i18n("Year"),  TimeLineWidget::Year);
    d->timeUnitCB->setCurrentIndex((int)TimeLineWidget::Month);
    d->timeUnitCB->setFocusPolicy(Qt::NoFocus);
    d->timeUnitCB->setWhatsThis(i18n("<p>Select the histogram time unit.</p>"
                                     "<p>You can change the graph decade to zoom in or zoom out over time.</p>"));

    QWidget* const scaleBox  = new QWidget(hbox1);
    QHBoxLayout* const hlay2 = new QHBoxLayout(scaleBox);
    d->scaleBG               = new QButtonGroup(scaleBox);
    d->scaleBG->setExclusive(true);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale.</p>"
                                 "<p>If the date's maximal counts are small, you can use the linear scale.</p>"
                                 "<p>Logarithmic scale can be used when the maximal counts are big; "
                                 "if it is used, all values (small and large) will be visible on the "
                                 "graph.</p>"));

    QToolButton* const linHistoButton = new QToolButton(scaleBox);
    linHistoButton->setToolTip(i18nc("@info: timeline sidebar", "Linear"));
    linHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-linear")));
    linHistoButton->setCheckable(true);
    d->scaleBG->addButton(linHistoButton, TimeLineWidget::LinScale);

    QToolButton* const logHistoButton = new QToolButton(scaleBox);
    logHistoButton->setToolTip(i18nc("@info: timeline sidebar", "Logarithmic"));
    logHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-logarithmic")));
    logHistoButton->setCheckable(true);
    d->scaleBG->addButton(logHistoButton, TimeLineWidget::LogScale);

    hlay2->setContentsMargins(QMargins());
    hlay2->setSpacing(0);
    hlay2->addWidget(linHistoButton);
    hlay2->addWidget(logHistoButton);

    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(spacing);
    hlay->addWidget(label1);
    hlay->addWidget(d->timeUnitCB);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlay->addWidget(scaleBox);

    // ---------------------------------------------------------------

    d->timeLineWidget = new TimeLineWidget(panel);
    d->scrollBar      = new QScrollBar(panel);
    d->scrollBar->setOrientation(Qt::Horizontal);
    d->scrollBar->setMinimum(0);
    d->scrollBar->setSingleStep(1);

    d->cursorDateLabel  = new DAdjustableLabel(panel);
    d->cursorCountLabel = new QLabel(panel);
    d->cursorCountLabel->setAlignment(Qt::AlignRight);

    // ---------------------------------------------------------------

    DHBox* const hbox2 = new DHBox(panel);
    hbox2->setContentsMargins(QMargins());
    hbox2->setSpacing(spacing);

    d->resetButton = new QToolButton(hbox2);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18n("Clear current selection"));
    d->resetButton->setWhatsThis(i18n("If you press this button, the current date selection on the time-line will be cleared."));
    d->nameEdit    = new DTextEdit(hbox2);
    d->nameEdit->setLinesVisible(1);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current dates search to save in the "
                                   "\"Searches\" view"));

    d->saveButton  = new QToolButton(hbox2);
    d->saveButton->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveButton->setEnabled(false);
    d->saveButton->setToolTip(i18n("Save current selection to a new virtual Album"));
    d->saveButton->setWhatsThis(i18n("If you press this button, the dates selected on the time-line will be "
                                     "saved to a new search virtual Album using the name set on the left."));

    // ---------------------------------------------------------------

    grid->addWidget(hbox1,               0, 0, 1, 4);
    grid->addWidget(d->cursorDateLabel,  1, 0, 1, 3);
    grid->addWidget(d->cursorCountLabel, 1, 3, 1, 1);
    grid->addWidget(d->timeLineWidget,   2, 0, 1, 4);
    grid->addWidget(d->scrollBar,        3, 0, 1, 4);
    grid->addWidget(hbox2,               4, 0, 1, 4);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // ---------------------------------------------------------------

    d->timeLineFolderView = new EditableSearchTreeView(this, searchModel, searchModificationHelper);
    d->timeLineFolderView->setConfigGroup(getConfigGroup());
    d->timeLineFolderView->filteredModel()->listTimelineSearches();
    d->timeLineFolderView->filteredModel()->setListTemporarySearches(false);
    d->timeLineFolderView->setAlbumManagerCurrentAlbum(false);
    d->searchDateBar      = new SearchTextBarDb(this, QLatin1String("TimeLineViewSearchDateBar"));
    d->searchDateBar->setModel(d->timeLineFolderView->filteredModel(),
                               AbstractAlbumModel::AlbumIdRole,
                               AbstractAlbumModel::AlbumTitleRole);
    d->searchDateBar->setFilterModel(d->timeLineFolderView->albumFilterModel());

    vlay->addWidget(panel);
    vlay->addWidget(d->timeLineFolderView);
    vlay->addItem(new QSpacerItem(spacing, spacing, QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlay->addWidget(d->searchDateBar);
    vlay->setContentsMargins(0, 0, spacing, 0);
    vlay->setSpacing(0);
    vlay->setStretchFactor(d->timeLineFolderView, 10);

    // ---------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalDatesMapDirty(QMap<QDateTime,int>)),
            d->timeLineWidget, SLOT(slotDatesMap(QMap<QDateTime,int>)));

    connect(d->timeLineFolderView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->timeUnitCB, SIGNAL(activated(int)),
            this, SLOT(slotTimeUnitChanged(int)));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->scaleBG, SIGNAL(idReleased(int)),
            this, SLOT(slotScaleChanged(int)));

#else

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

#endif

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalCursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalRefDateTimeChanged()),
            this, SLOT(slotRefDateTimeChanged()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdateCurrentDateSearchAlbum()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetSelection()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveSelection()));

    connect(d->scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(slotScrollBarValueChanged(int)));

    connect(d->nameEdit, SIGNAL(textChanged()),
            this, SLOT(slotCheckAboutSelection()));

    connect(d->nameEdit, SIGNAL(returnPressed()),
            d->saveButton, SLOT(animateClick()));
}

TimelineSideBarWidget::~TimelineSideBarWidget()
{
    delete d;
}

void TimelineSideBarWidget::slotInit()
{
    // Date Maps are loaded from AlbumManager to TimeLineWidget after than GUI is initialized.
    // AlbumManager query Date KIO slave to stats items from database and it can take a while.
    // We waiting than TimeLineWidget is ready before to set last config from users.

    loadState();

    disconnect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
               this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotCursorPositionChanged()));
}

void TimelineSideBarWidget::setActive(bool active)
{
    if (active)
    {
        if (!d->currentTimelineSearch)
        {
            d->currentTimelineSearch = d->timeLineFolderView->currentAlbum();
        }

        if (d->currentTimelineSearch)
        {
            AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->currentTimelineSearch);
        }
        else
        {
            slotUpdateCurrentDateSearchAlbum();
        }
    }
}

void TimelineSideBarWidget::doLoadState()
{

    KConfigGroup group = getConfigGroup();

    d->timeUnitCB->setCurrentIndex(group.readEntry(d->configHistogramTimeUnitEntry, (int)TimeLineWidget::Month));
    slotTimeUnitChanged(d->timeUnitCB->currentIndex());

    int id = group.readEntry(d->configHistogramScaleEntry, (int)TimeLineWidget::LinScale);

    if (d->scaleBG->button(id))
    {
        d->scaleBG->button(id)->setChecked(true);
    }

    slotScaleChanged(d->scaleBG->checkedId());

    QDateTime now = QDateTime::currentDateTime();
    d->timeLineWidget->setCursorDateTime(group.readEntry(d->configCursorPositionEntry, now));
    d->timeLineWidget->setCurrentIndex(d->timeLineWidget->indexForCursorDateTime());

    d->timeLineFolderView->loadState();
}

void TimelineSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(d->configHistogramTimeUnitEntry, d->timeUnitCB->currentIndex());
    group.writeEntry(d->configHistogramScaleEntry,    d->scaleBG->checkedId());
    group.writeEntry(d->configCursorPositionEntry,    d->timeLineWidget->cursorDateTime());

    d->timeLineFolderView->saveState();

    group.sync();
}

void TimelineSideBarWidget::applySettings()
{
    // nothing to do here right now
}

void TimelineSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    d->timeLineFolderView->setCurrentAlbums(album);
}

const QIcon TimelineSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("player-time"));
}

const QString TimelineSideBarWidget::getCaption()
{
    return i18n("Timeline");
}

void TimelineSideBarWidget::slotRefDateTimeChanged()
{
    d->scrollBar->blockSignals(true);

    int totalIndex = d->timeLineWidget->totalIndex()-1;

    if ((totalIndex >= 0) && (d->scrollBar->maximum() != totalIndex))
    {
        d->scrollBar->setMaximum(totalIndex);
        d->scrollBar->setValue(d->timeLineWidget->indexForRefDateTime()-1);
    }

    d->scrollBar->blockSignals(false);
}

void TimelineSideBarWidget::slotTimeUnitChanged(int mode)
{
    d->timeLineWidget->setTimeUnit((TimeLineWidget::TimeUnit)mode);
}

void TimelineSideBarWidget::slotScrollBarValueChanged(int val)
{
    d->timeLineWidget->setCurrentIndex(val);
}

void TimelineSideBarWidget::slotScaleChanged(int mode)
{
    d->timeLineWidget->setScaleMode((TimeLineWidget::ScaleMode)mode);
}

void TimelineSideBarWidget::slotCursorPositionChanged()
{
    QString txt;
    int val = d->timeLineWidget->cursorInfo(txt);
    d->cursorDateLabel->setAdjustedText(txt);
    d->cursorCountLabel->setText((val == 0) ? i18n("no item")
                                            : i18np("1 item", "%1 items", val));
}

void TimelineSideBarWidget::slotSelectionChanged()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

/**
 * Called from d->timer event.
 */
void TimelineSideBarWidget::slotUpdateCurrentDateSearchAlbum()
{
    slotCheckAboutSelection();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->currentTimelineSearch = d->searchModificationHelper->
        slotCreateTimeLineSearch(SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch), dateRanges, true);

    // NOTE: "temporary" search is not listed in view

    d->timeLineFolderView->setCurrentAlbum(0);
}

void TimelineSideBarWidget::slotSaveSelection()
{
    QString name             = d->nameEdit->text();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->currentTimelineSearch = d->searchModificationHelper->slotCreateTimeLineSearch(name, dateRanges);
}

void TimelineSideBarWidget::slotAlbumSelected(Album* album)
{
    if (d->currentTimelineSearch == album)
    {
        return;
    }

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum)
    {
        return;
    }

    d->currentTimelineSearch = salbum;
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    SearchXmlReader reader(salbum->query());

    // The timeline query consists of groups, with two date time fields each

    DateRangeList list;

    while (!reader.atEnd())
    {
        // read groups

        if (reader.readNext() == SearchXml::Group)
        {
            QDateTime start, end;
            int numberOfFields = 0;

            while (!reader.atEnd())
            {
                // read fields

                reader.readNext();

                if (reader.isEndElement())
                {
                    break;
                }

                if (reader.isFieldElement())
                {
                    if (numberOfFields == 0)
                    {
                        start = reader.valueToDateTime();
                    }
                    else if (numberOfFields == 1)
                    {
                        end = reader.valueToDateTime();
                    }

                    ++numberOfFields;
                }
            }

            if (numberOfFields)
            {
                list << DateRange(start, end);
            }
        }
    }

    d->timeLineWidget->setSelectedDateRange(list);
}

void TimelineSideBarWidget::slotResetSelection()
{
    d->timeLineWidget->slotResetSelection();
    slotCheckAboutSelection();
    AlbumManager::instance()->clearCurrentAlbums();
}

void TimelineSideBarWidget::slotCheckAboutSelection()
{
    int totalCount     = 0;
    DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);

    if (!list.isEmpty())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
        {
            d->saveButton->setEnabled(true);
        }
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveButton->setEnabled(false);
    }
}

} // namespace Digikam

#include "moc_timelinesidebarwidget.cpp"
