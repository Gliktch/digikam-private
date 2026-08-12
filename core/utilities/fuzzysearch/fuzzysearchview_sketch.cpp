/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents - sketch panel.
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fuzzysearchview_p.h"

namespace Digikam
{

QWidget* FuzzySearchView::setupSketchPanel() const
{
    const int spacing       = layoutSpacing();

    DHBox* const drawingBox = new DHBox();
    d->sketchWidget         = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QString tooltip(i18n("Set here the brush color used to draw sketch."));

    d->hsSelector = new DHueSaturationSelector();
    d->hsSelector->setMinimumSize(200, 96);
    d->hsSelector->setChooserMode(ChooserValue);
    d->hsSelector->setColorValue(255);
    d->hsSelector->setWhatsThis(tooltip);

    d->vSelector  = new DColorValueSelector();
    d->vSelector->setMinimumSize(26, 96);
    d->vSelector->setChooserMode(ChooserValue);
    d->vSelector->setIndent(false);
    d->vSelector->setWhatsThis(tooltip);

    // ---------------------------------------------------------------

    d->undoBtnSketch   = new QToolButton();
    d->undoBtnSketch->setAutoRepeat(true);
    d->undoBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    d->undoBtnSketch->setToolTip(i18n("Undo last draw on sketch"));
    d->undoBtnSketch->setWhatsThis(i18n("Use this button to undo last drawing action on sketch."));
    d->undoBtnSketch->setEnabled(false);

    d->redoBtnSketch   = new QToolButton();
    d->redoBtnSketch->setAutoRepeat(true);
    d->redoBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("edit-redo")));
    d->redoBtnSketch->setToolTip(i18n("Redo last draw on sketch"));
    d->redoBtnSketch->setWhatsThis(i18n("Use this button to redo last drawing action on sketch."));
    d->redoBtnSketch->setEnabled(false);

    QLabel* const brushLabel = new QLabel(i18n("Pen:"));
    d->penSize               = new QSpinBox();
    d->penSize->setRange(1, 64);
    d->penSize->setSingleStep(1);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("Set here the brush size in pixels used to draw sketch."));

    QLabel* const resultsLabel = new QLabel(i18n("Items:"));
    d->resultsSketch           = new QSpinBox();
    d->resultsSketch->setRange(1, 50);
    d->resultsSketch->setSingleStep(1);
    d->resultsSketch->setValue(10);
    d->resultsSketch->setWhatsThis(i18n("Set here the number of items to find using sketch."));

    QGridLayout* const settingsLayout = new QGridLayout();
    settingsLayout->addWidget(d->undoBtnSketch, 0, 0);
    settingsLayout->addWidget(d->redoBtnSketch, 0, 1);
    settingsLayout->addWidget(brushLabel,       0, 2);
    settingsLayout->addWidget(d->penSize,       0, 3);
    settingsLayout->addWidget(resultsLabel,     0, 5);
    settingsLayout->addWidget(d->resultsSketch, 0, 6);
    settingsLayout->setColumnStretch(4, 10);
    settingsLayout->setContentsMargins(QMargins());
    settingsLayout->setSpacing(spacing);

    // ---------------------------------------------------------------

    d->sketchSearchAlbumSelectors = new AlbumSelectors(i18nc("@label",
                                                       "Search in albums:"),
                                                       QLatin1String("Sketch Search View"),
                                                       nullptr, AlbumSelectors::AlbumType::PhysAlbum, true);

    // ---------------------------------------------------------------

    DHBox* const saveBox = new DHBox();
    saveBox->setContentsMargins(QMargins());
    saveBox->setSpacing(spacing);

    d->resetButton = new QToolButton(saveBox);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("Use this button to clear sketch contents."));

    d->nameEditSketch = new DTextEdit(saveBox);
    d->nameEditSketch->setLinesVisible(1);
    d->nameEditSketch->setWhatsThis(i18n("Enter the name of the current sketch search to save in the "
                                         "\"Similarity Searches\" view."));

    d->saveBtnSketch = new QToolButton(saveBox);
    d->saveBtnSketch->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveBtnSketch->setEnabled(false);
    d->saveBtnSketch->setToolTip(i18n("Save current sketch search to a new virtual Album"));
    d->saveBtnSketch->setWhatsThis(i18n("If you press this button, the current sketch "
                                        "fuzzy search will be saved to a new search "
                                        "virtual album using the name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget;
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(drawingBox,                    0, 0, 1, 3);
    mainLayout->addWidget(d->hsSelector,                 1, 0, 1, 2);
    mainLayout->addWidget(d->vSelector,                  1, 2, 1, 1);
    mainLayout->addLayout(settingsLayout,                2, 0, 1, 3);
    mainLayout->addWidget(d->sketchSearchAlbumSelectors, 3, 0, 1, 3);
    mainLayout->addWidget(saveBox,                       4, 0, 1, 3);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

void FuzzySearchView::slotHSChanged(int h, int s)
{
    QColor color;

    int val = d->selColor.value();

    color.setHsv(h, s, val);
    setColor(color);
}

void FuzzySearchView::slotVChanged(int v)
{
    QColor color;

    int hue = d->selColor.hue();
    int sat = d->selColor.saturation();

    color.setHsv(hue, sat, v);
    setColor(color);
}

void FuzzySearchView::slotPenColorChanged(const QColor& color)
{
    slotHSChanged(color.hue(), color.saturation());
    slotVChanged(color.value());
}

void FuzzySearchView::setColor(QColor c)
{
    if (c.isValid())
    {
        d->selColor = c;

        // set values

        d->hsSelector->setValues(c.hue(), c.saturation());
        d->vSelector->setValue(c.value());

        // set colors

        d->hsSelector->blockSignals(true);
        d->hsSelector->setHue(c.hue());
        d->hsSelector->setSaturation(c.saturation());
        d->hsSelector->setColorValue(c.value());
        d->hsSelector->updateContents();
        d->hsSelector->blockSignals(false);
        d->hsSelector->repaint();

        d->vSelector->blockSignals(true);
        d->vSelector->setHue(c.hue());
        d->vSelector->setSaturation(c.saturation());
        d->vSelector->setColorValue(c.value());
        d->vSelector->updateContents();
        d->vSelector->blockSignals(false);
        d->vSelector->repaint();

        d->sketchWidget->setPenColor(c);
    }
}

void FuzzySearchView::slotUndoRedoStateChanged(bool hasUndo, bool hasRedo)
{
    d->undoBtnSketch->setEnabled(hasUndo);
    d->redoBtnSketch->setEnabled(hasRedo);
}

void FuzzySearchView::slotSaveSketchSAlbum()
{
    createNewFuzzySearchAlbumFromSketch(d->nameEditSketch->text());
}

void FuzzySearchView::slotDirtySketch()
{
    if (d->active)
    {
        d->timerSketch->start();
    }
}

void FuzzySearchView::slotTimerSketchDone()
{
    slotCheckNameEditSketchConditions();
    createNewFuzzySearchAlbumFromSketch(SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch), true);
}

void FuzzySearchView::createNewFuzzySearchAlbumFromSketch(const QString& name, bool force)
{
    AlbumManager::instance()->clearCurrentAlbums();

    QList<int> albums = d->sketchSearchAlbumSelectors->selectedAlbumIds();

    d->sketchSAlbum = d->searchModificationHelper->createFuzzySearchFromSketch(name, d->sketchWidget,
                                                                               d->resultsSketch->value(),
                                                                               albums, force);
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->sketchSAlbum);
}

void FuzzySearchView::slotClearSketch()
{
    d->sketchWidget->slotClear();
    slotCheckNameEditSketchConditions();
    AlbumManager::instance()->clearCurrentAlbums();
}

void FuzzySearchView::slotCheckNameEditSketchConditions()
{
    if (!d->sketchWidget->isClear())
    {
        bool b = d->nameEditSketch->text().isEmpty();
        d->nameEditSketch->setEnabled(true);
        d->saveBtnSketch->setEnabled(!b);
    }
    else
    {
        d->nameEditSketch->setEnabled(false);
        d->saveBtnSketch->setEnabled(false);
    }
}

} // namespace Digikam
