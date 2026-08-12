/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-10
 * Description : a tool bar for preview mode
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "previewtoolbar.h"

// Qt includes

#include <QButtonGroup>
#include <QLayout>
#include <QToolButton>
#include <QAbstractButton>
#include <QStandardPaths>
#include <QActionGroup>
#include <QMenu>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "editorwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN PreviewToolBar::Private
{
public:

    Private() = default;

    QToolButton*  previewOriginalButton             = nullptr;
    QToolButton*  previewBothButtonVert             = nullptr;
    QToolButton*  previewBothButtonHorz             = nullptr;
    QToolButton*  previewDuplicateBothButtonVert    = nullptr;
    QToolButton*  previewDupplicateBothButtonHorz   = nullptr;
    QToolButton*  previewtargetButton               = nullptr;
    QToolButton*  previewToggleMouseOverButton      = nullptr;

    QButtonGroup* previewButtons                    = nullptr;
    QActionGroup* actionsGroup                      = nullptr;

    QMenu*        actionsMenu                       = nullptr;
};

PreviewToolBar::PreviewToolBar(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout* const hlay = new QHBoxLayout(this);
    d->previewButtons       = new QButtonGroup(this);
    d->previewButtons->setExclusive(true);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);

    d->previewOriginalButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewOriginalButton, PreviewOriginalImage);
    hlay->addWidget(d->previewOriginalButton);
    d->previewOriginalButton->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/original.png"))));
    d->previewOriginalButton->setCheckable(true);
    d->previewOriginalButton->setWhatsThis(i18n("If this option is enabled, the original image will be shown."));
    d->previewOriginalButton->setToolTip(i18n("Preview original image"));
    d->previewOriginalButton->setObjectName(QLatin1String("preview-original"));

    d->previewBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(d->previewBothButtonVert, PreviewBothImagesVertCont);
    hlay->addWidget(d->previewBothButtonVert);
    d->previewBothButtonVert->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/bothvert.png"))));
    d->previewBothButtonVert->setCheckable(true);
    d->previewBothButtonVert->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                "split vertically. "
                                                "A contiguous area of the image will be shown, "
                                                "with one half from the original image, "
                                                "the other half from the target image."));
    d->previewBothButtonVert->setToolTip(i18n("Preview vertical split with contiguous image"));
    d->previewBothButtonVert->setObjectName(QLatin1String("preview-both-vert"));

    d->previewBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(d->previewBothButtonHorz, PreviewBothImagesHorzCont);
    hlay->addWidget(d->previewBothButtonHorz);
    d->previewBothButtonHorz->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/bothhorz.png"))));
    d->previewBothButtonHorz->setCheckable(true);
    d->previewBothButtonHorz->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                "split horizontally. "
                                                "A contiguous area of the image will be shown, "
                                                "with one half from the original image, "
                                                "the other half from the target image."));
    d->previewBothButtonHorz->setToolTip(i18n("Preview horizontal split with contiguous image"));
    d->previewBothButtonHorz->setObjectName(QLatin1String("preview--both-horz"));

    d->previewDuplicateBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(d->previewDuplicateBothButtonVert, PreviewBothImagesVert);
    hlay->addWidget(d->previewDuplicateBothButtonVert);
    d->previewDuplicateBothButtonVert->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/duplicatebothvert.png"))));
    d->previewDuplicateBothButtonVert->setCheckable(true);
    d->previewDuplicateBothButtonVert->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                         "split vertically. "
                                                         "The same part of the original and the target image "
                                                         "will be shown side by side."));
    d->previewDuplicateBothButtonVert->setToolTip(i18n("Preview vertical split with same image region"));
    d->previewDuplicateBothButtonVert->setObjectName(QLatin1String("preview-duplicate-both-vert"));

    d->previewDupplicateBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(d->previewDupplicateBothButtonHorz, PreviewBothImagesHorz);
    hlay->addWidget(d->previewDupplicateBothButtonHorz);
    d->previewDupplicateBothButtonHorz->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/duplicatebothhorz.png"))));
    d->previewDupplicateBothButtonHorz->setCheckable(true);
    d->previewDupplicateBothButtonHorz->setWhatsThis(i18n("If this option is enabled, the preview area will "
                                                          "split horizontally. "
                                                          "The same part of the original and the target image "
                                                          "will be shown side by side."));
    d->previewDupplicateBothButtonHorz->setToolTip(i18n("Preview horizontal split with same image region"));
    d->previewDupplicateBothButtonHorz->setObjectName(QLatin1String("preview-duplicate-both-horz"));

    d->previewtargetButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewtargetButton, PreviewTargetImage);
    hlay->addWidget(d->previewtargetButton);
    d->previewtargetButton->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/target.png"))));
    d->previewtargetButton->setCheckable(true);
    d->previewtargetButton->setWhatsThis(i18n("If this option is enabled, the target image will be shown."));
    d->previewtargetButton->setToolTip(i18n("Preview target image"));
    d->previewtargetButton->setObjectName(QLatin1String("preview-target"));

    d->previewToggleMouseOverButton = new QToolButton(this);
    d->previewButtons->addButton(d->previewToggleMouseOverButton, PreviewToggleOnMouseOver);
    hlay->addWidget(d->previewToggleMouseOverButton);
    d->previewToggleMouseOverButton->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/togglemouseover.png"))));
    d->previewToggleMouseOverButton->setCheckable(true);
    d->previewToggleMouseOverButton->setWhatsThis(i18n("If this option is enabled, the original image will "
                                                       "be shown when the mouse is over image area; otherwise, "
                                                       "the target image will be shown."));
    d->previewToggleMouseOverButton->setToolTip(i18n("Preview on mouse-over"));
    d->previewToggleMouseOverButton->setObjectName(QLatin1String("preview-toggle-mouse-over"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->previewButtons, SIGNAL(idReleased(int)),
            this, SLOT(slotButtonReleased(int)));

#else

    connect(d->previewButtons, SIGNAL(buttonReleased(int)),
            this, SLOT(slotButtonReleased(int)));

#endif

}

PreviewToolBar::~PreviewToolBar()
{
    delete d;
}

void PreviewToolBar::registerMenuActionGroup(EditorWindow* const editor)
{
    d->actionsMenu  = new QMenu(i18nc("@action Select image editor preview mode",
                                      "Preview Mode"), editor);
    d->actionsGroup = new QActionGroup(d->actionsMenu);

    connect(d->actionsGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotActionTriggered(QAction*)));

    const auto btns = d->previewButtons->buttons();

    for (QAbstractButton* const btn : btns)
    {
        QAction* const ac = new QAction(btn->toolTip(), d->actionsGroup);
        ac->setData(QVariant(d->previewButtons->id(btn)));
        ac->setIcon(btn->icon());
        ac->setCheckable(true);
        editor->actionCollection()->addAction(btn->objectName(), ac);
        d->actionsMenu->addAction(ac);
    }

    editor->actionCollection()->addAction(QLatin1String("editorwindow_previewmode"),
                                          d->actionsMenu->menuAction());
}

void PreviewToolBar::slotActionTriggered(QAction* ac)
{
    int id = ac->data().toInt();
    d->previewButtons->button(id)->setChecked(true);

    Q_EMIT signalPreviewModeChanged(id);
}

void PreviewToolBar::slotButtonReleased(int id)
{
    setCheckedAction(id);

    Q_EMIT signalPreviewModeChanged(id);
}

void PreviewToolBar::setCheckedAction(int id)
{
    if (!d->actionsGroup)
    {
        return;
    }

    const auto acs = d->actionsGroup->actions();

    for (QAction* const ac : acs)
    {
        if (ac->data().toInt() == id)
        {   // cppcheck-suppress useStlAlgorithm
            ac->setChecked(true);
            return;
        }
    }
}

void PreviewToolBar::setPreviewModeMask(int mask)
{
    if (mask == NoPreviewMode)
    {
        setDisabled(true);

        if (d->actionsMenu)
        {
            d->actionsMenu->setDisabled(true);
        }

        return;
    }

    setDisabled(false);

    if (d->actionsMenu)
    {
        d->actionsMenu->setDisabled(false);
    }

    d->previewOriginalButton->setEnabled(mask           & PreviewOriginalImage);
    d->previewBothButtonVert->setEnabled(mask           & PreviewBothImagesHorz);
    d->previewBothButtonHorz->setEnabled(mask           & PreviewBothImagesVert);
    d->previewDuplicateBothButtonVert->setEnabled(mask  & PreviewBothImagesHorzCont);
    d->previewDupplicateBothButtonHorz->setEnabled(mask & PreviewBothImagesVertCont);
    d->previewtargetButton->setEnabled(mask             & PreviewTargetImage);
    d->previewToggleMouseOverButton->setEnabled(mask    & PreviewToggleOnMouseOver);

    if (d->actionsGroup)
    {
        const auto acs = d->actionsGroup->actions();

        for (QAction* const ac : acs)
        {
            ac->setEnabled(mask & ac->data().toInt());
        }
    }

    // When we switch to another mask, check if current mode is valid.

    PreviewToolBar::PreviewMode mode = previewMode();

    if (d->previewButtons->button(mode))
    {
        if (!d->previewButtons->button(mode)->isEnabled())
        {
            QList<QAbstractButton*> btns = d->previewButtons->buttons();

            for (QAbstractButton* const btn : std::as_const(btns))
            {
                if (btn && btn->isEnabled())
                {   // cppcheck-suppress useStlAlgorithm
                    btn->setChecked(true);
                    setCheckedAction(d->previewButtons->id(btn));

                    return;
                }
            }
        }
    }
}

void PreviewToolBar::setPreviewMode(PreviewMode mode)
{
    if (d->previewButtons->button(mode))
    {
        d->previewButtons->button(mode)->setChecked(true);
        setCheckedAction((int)mode);
    }
}

PreviewToolBar::PreviewMode PreviewToolBar::previewMode() const
{
    if (!isEnabled())
    {
        return PreviewToolBar::NoPreviewMode;
    }

    return ((PreviewMode)d->previewButtons->checkedId());
}

void PreviewToolBar::readSettings(const KConfigGroup& group)
{
    int mode = group.readEntry("PreviewMode", (int)PreviewBothImagesVertCont);
    mode     = qMax((int)PreviewOriginalImage, mode);
    mode     = qMin((int)PreviewToggleOnMouseOver, mode);
    setPreviewMode((PreviewMode)mode);
}

void PreviewToolBar::writeSettings(KConfigGroup& group)
{
    group.writeEntry("PreviewMode", (int)previewMode());
}

} // namespace Digikam

#include "moc_previewtoolbar.cpp"
