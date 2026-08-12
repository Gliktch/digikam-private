/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmisc_p.h"

namespace Digikam
{

void SetupMisc::setupBehavior()
{
    // -- Application Behavior Options --------------------------------------------------------

    QWidget* const behaviorPanel      = new QWidget(d->tab);
    QGridLayout* const layout         = new QGridLayout(behaviorPanel);

    QLabel* const startupNote = new QLabel(i18nc("@info",
                                                 "<font color='black'><i><b>⚠️Warning:</b> "
                                                 "These settings make startup slower! Read the "
                                                 "<a href=\"%1\">online documentation</a> carefully first.</i></font>",
                                                 QString::fromUtf8("https://docs.digikam.org/en/setup_application/miscs_settings.html#behavior-settings")),
                                           this);
    startupNote->setOpenExternalLinks(true);
    startupNote->setTextFormat(Qt::RichText);
    startupNote->setFocusPolicy(Qt::NoFocus);
    startupNote->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    startupNote->setWordWrap(true);
    startupNote->setAlignment(Qt::AlignCenter);
    startupNote->setStyleSheet(QLatin1String("QLabel { background-color: #ffcccc; border: none; padding: 5px; }"));

    d->scanAtStart                    = new QCheckBox(i18n("&Scan for new items at startup"), behaviorPanel);
    d->scanAtStart->setToolTip(i18n("Set this option to force digiKam to scan all collections for new items to\n"
                                    "register new elements in database. The scan is performed in the background through\n"
                                    "the progress manager available in the statusbar\n when digiKam main interface\n"
                                    "is loaded. If your computer is fast enough, this will have no effect on usability\n"
                                    "of digiKam while scanning. If your collections are huge or if you use a remote database,\n"
                                    "this can introduce low latency, and it is recommended to disable this option and to plan\n"
                                    "a manual scan through the maintenance tool at the right moment."));

    d->useFastScan                    = new QCheckBox(i18n("Fast scan (detects new, deleted and renamed items)"), behaviorPanel);
    d->useFastScan->setToolTip(i18n("Set this option to significantly speed up the scan. New items, deleted and also\n"
                                    "renamed items are found. In order to find items that have been changed, this\n"
                                    "option must be deactivated."));

    d->useFastScan->setEnabled(false);

    d->detectFaces                    = new QCheckBox(i18n("Detect faces in newly added images"), behaviorPanel);
    d->faceRecognitionBackgroundScan  = new QCheckBox(i18n("Enable background face recognition scan"), behaviorPanel);
    d->faceRecognitionBackgroundScan->setToolTip(i18n("Set this option to automatically start a background face recognition scan\n"
                                                     "when a new face is tagged. This will make the tagging process easier."));

    d->cleanAtStart                   = new QCheckBox(i18n("Remove obsolete core database objects"), behaviorPanel);
    d->cleanAtStart->setToolTip(i18n("Set this option to force digiKam to clean up the core database from obsolete item entries.\n"
                                     "Entries are only deleted if the connected image/video/audio file was already removed, i.e.\n"
                                     "the database object wastes space.\n"
                                     "This option does not clean up other databases as the thumbnails or recognition db.\n"
                                     "For clean up routines for other databases, please use the maintenance."));

    // ---------------------------------------------------------

    d->showTrashDeleteDialogCheck             = new QCheckBox(i18n("Confirm when moving items to the &trash"), behaviorPanel);
    d->showPermanentDeleteDialogCheck         = new QCheckBox(i18n("Confirm when permanently deleting items"), behaviorPanel);
    d->sidebarApplyDirectlyCheck              = new QCheckBox(i18n("Do not confirm when applying changes in the &right sidebar"), behaviorPanel);
    d->showOnlyPersonTagsInPeopleSidebarCheck = new QCheckBox(i18n("Show only face tags for assigning names in people sidebar"), behaviorPanel);
    d->selectFirstAlbumItemCheck              = new QCheckBox(i18n("Initially select the first item in the album"), behaviorPanel);
    d->expandNewCurrentItemCheck              = new QCheckBox(i18n("Expand current tree item with a single mouse click"), behaviorPanel);
    d->scrollItemToCenterCheck                = new QCheckBox(i18n("Scroll current item to center of thumbbar"), behaviorPanel);

    // ---------------------------------------------------------

    DHBox* const albumDateSourceHbox  = new DHBox(behaviorPanel);
    d->albumDateSourceLabel           = new QLabel(i18n("Get album date source:"), albumDateSourceHbox);
    d->albumDateSource                = new QComboBox(albumDateSourceHbox);
    d->albumDateSource->addItem(i18nc("method to get the album date", "From The Newest Item Date"), MetaEngineSettingsContainer::NewestItemDate);
    d->albumDateSource->addItem(i18nc("method to get the album date", "From The Oldest Item Date"), MetaEngineSettingsContainer::OldestItemDate);
    d->albumDateSource->addItem(i18nc("method to get the album date", "From The Average Date"),     MetaEngineSettingsContainer::AverageDate);
    d->albumDateSource->addItem(i18nc("method to get the album date", "From The Folder Date"),      MetaEngineSettingsContainer::FolderDate);
    d->albumDateSource->addItem(i18nc("method to get the album date", "No Change Of Date"),         MetaEngineSettingsContainer::IgnoreDate);
    d->albumDateSource->setToolTip(i18n("The album date is adjusted depending on the option set "
                                        "when new items are found during scanning."));

    DHBox* const stringComparisonHbox = new DHBox(behaviorPanel);
    d->stringComparisonTypeLabel      = new QLabel(i18n("String comparison type:"), stringComparisonHbox);
    d->stringComparisonType           = new QComboBox(stringComparisonHbox);
    d->stringComparisonType->addItem(i18nc("method to compare strings", "Natural"), ApplicationSettings::Natural);
    d->stringComparisonType->addItem(i18nc("method to compare strings", "Normal"),  ApplicationSettings::Normal);
    d->stringComparisonType->setToolTip(i18n("<qt>Sets the way in which strings are compared inside digiKam. "
                                             "This eg. influences the sorting of the tree views.<br>"
                                             "<b>Natural</b> tries to compare strings in a way that regards some normal conventions "
                                             "and will eg. result in sorting numbers naturally even if they have a different number of digits.<br>"
                                             "<b>Normal</b> uses a more technical approach. "
                                             "Use this style if you eg. want to entitle albums with ISO dates (201006 or 20090523) "
                                             "and the albums should be sorted according to these dates.</qt>"));

    DHBox* const minSimilarityBoundHbox       = new DHBox(behaviorPanel);
    d->minSimilarityBoundLabel                = new QLabel(i18n("Lower bound for minimum similarity:"), minSimilarityBoundHbox);
    d->minimumSimilarityBound                 = new QSpinBox(minSimilarityBoundHbox);
    d->minimumSimilarityBound->setSuffix(QLatin1String("%"));
    d->minimumSimilarityBound->setRange(1, 100);
    d->minimumSimilarityBound->setSingleStep(1);
    d->minimumSimilarityBound->setValue(40);
    d->minimumSimilarityBound->setToolTip(i18n("Select here the lower bound of "
                                               "the minimum similarity threshold "
                                               "for fuzzy and duplicates searches. "
                                               "The default value is 40. Selecting "
                                               "a lower value than 40 can make the search <b>really</b> slow."));
    d->minSimilarityBoundLabel->setBuddy(d->minimumSimilarityBound);

    // ---------------------------------------------------------

    QGroupBox* const upOptionsGroup = new QGroupBox(i18n("Updates"), behaviorPanel);
    QVBoxLayout* const gLayout5     = new QVBoxLayout();

    DHBox* const updateHbox      = new DHBox(upOptionsGroup);
    d->updateTypeLabel           = new QLabel(i18n("Check for new version:"), updateHbox);
    d->updateType                = new QComboBox(updateHbox);
    d->updateType->addItem(i18n("Only For Stable Releases"), 0);
    d->updateType->addItem(i18n("Weekly Pre-Releases"),      1);
    d->updateType->setToolTip(i18n("Set this option to configure which kind of new versions must be check for updates.\n"
                                   "\"Stable\" releases are official versions safe to use in production.\n"
                                   "\"Pre-releases\" are proposed weekly to tests quickly new features\n"
                                   "and are not recommended to use in production as bugs can remain."));

    d->updateWithDebug           = new QCheckBox(i18n("Use Version With Debug Symbols"), upOptionsGroup);
    d->updateWithDebug->setWhatsThis(i18n("If this option is enabled, a version including debug symbols will be used for updates.\n"
                                          "This version is more heavy but can help developers to trace dysfunctions in debugger."));

    DHBox* const updateHbox2     = new DHBox(upOptionsGroup);
    QLabel* const lastCheckLabel = new QLabel(updateHbox2);
    lastCheckLabel->setText(i18n("Last check: %1", OnlineVersionChecker::lastCheckDate()));
    QPushButton* const updateNow = new QPushButton(i18n("Check now..."), updateHbox2);

    connect(updateNow, &QPushButton::pressed,
            this, [this]()
        {
            if (!checkSettings())
            {
                return;
            }

            applySettings();

            if (d->parent)
            {
                d->parent->close();
            }

            Setup::onlineVersionCheck();
        }
    );

    gLayout5->addWidget(updateHbox);
    gLayout5->addWidget(d->updateWithDebug);
    gLayout5->addWidget(updateHbox2);
    upOptionsGroup->setLayout(gLayout5);

    // ---------------------------------------------------------

    layout->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    layout->setSpacing(d->spacing);
    layout->addWidget(startupNote,                                0, 0, 1, 4);
    layout->addWidget(d->scanAtStart,                             1, 0, 1, 4);
    layout->addWidget(d->useFastScan,                             2, 3, 1, 1);
    layout->addWidget(d->detectFaces,                             3, 0, 1, 4);
    layout->addWidget(d->faceRecognitionBackgroundScan,           4, 0, 1, 4);
    layout->addWidget(d->cleanAtStart,                            5, 0, 1, 4);
    layout->addWidget(new DLineWidget(Qt::Horizontal),            6, 0, 1, 4);
    layout->addWidget(d->showTrashDeleteDialogCheck,              7, 0, 1, 4);
    layout->addWidget(d->showPermanentDeleteDialogCheck,          8, 0, 1, 4);
    layout->addWidget(d->sidebarApplyDirectlyCheck,               9, 0, 1, 4);
    layout->addWidget(new DLineWidget(Qt::Horizontal),           10, 0, 1, 4);
    layout->addWidget(d->showOnlyPersonTagsInPeopleSidebarCheck, 11, 0, 1, 4);
    layout->addWidget(d->selectFirstAlbumItemCheck,              12, 0, 1, 4);
    layout->addWidget(d->expandNewCurrentItemCheck,              13, 0, 1, 4);
    layout->addWidget(d->scrollItemToCenterCheck,                14, 0, 1, 4);
    layout->addWidget(new DLineWidget(Qt::Horizontal),           15, 0, 1, 4);
    layout->addWidget(albumDateSourceHbox,                       16, 0, 1, 4);
    layout->addWidget(stringComparisonHbox,                      17, 0, 1, 4);
    layout->addWidget(minSimilarityBoundHbox,                    18, 0, 1, 4);
    layout->addWidget(upOptionsGroup,                            19, 0, 1, 4);
    layout->setColumnStretch(3, 10);
    layout->setRowStretch(20, 10);

    // --------------------------------------------------------

    connect(d->scanAtStart, SIGNAL(toggled(bool)),
            d->useFastScan, SLOT(setEnabled(bool)));

    connect(d->faceRecognitionBackgroundScan, SIGNAL(toggled(bool)),
            FaceRecognitionBackgroundController::instance(), SLOT(slotSetEnabled(bool)));

    // --------------------------------------------------------

    d->tab->insertTab(Behavior, behaviorPanel, i18nc("@title:tab", "Behavior"));
}

} // namespace Digikam
