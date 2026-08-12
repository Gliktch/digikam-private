/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-13
 * Description : a widget to provide conflict rules to save image.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filesaveconflictbox.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN FileSaveConflictBox::Private
{
public:

    Private() = default;

public:

    QLabel*       conflictLabel         = nullptr;

    QButtonGroup* conflictButtonGroup   = nullptr;

    QRadioButton* storeDiffButton       = nullptr;
    QRadioButton* overwriteButton       = nullptr;
    QRadioButton* skipFileButton        = nullptr;
};

FileSaveConflictBox::FileSaveConflictBox(QWidget* const parent, bool addSkip)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing          = layoutSpacing();

    QGridLayout* const grid    = new QGridLayout(this);
    d->conflictLabel           = new QLabel(i18n("If Target File Exists:"), this);
    QWidget* const conflictBox = new QWidget(this);
    QVBoxLayout* const vlay    = new QVBoxLayout(conflictBox);
    d->conflictButtonGroup     = new QButtonGroup(conflictBox);
    d->storeDiffButton         = new QRadioButton(i18n("Store as a different name"), conflictBox);
    d->overwriteButton         = new QRadioButton(i18n("Overwrite automatically"),   conflictBox);
    d->skipFileButton          = new QRadioButton(i18n("Skip automatically"),        conflictBox);

    d->conflictButtonGroup->addButton(d->overwriteButton, OVERWRITE);
    d->conflictButtonGroup->addButton(d->storeDiffButton, DIFFNAME);
    d->conflictButtonGroup->addButton(d->skipFileButton,  SKIPFILE);

    d->conflictButtonGroup->setExclusive(true);
    d->storeDiffButton->setChecked(true);

    if (!addSkip)
    {
        d->skipFileButton->hide();
    }

    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);
    vlay->addWidget(d->storeDiffButton);
    vlay->addWidget(d->overwriteButton);
    vlay->addWidget(d->skipFileButton);

    grid->addWidget(d->conflictLabel, 1, 0, 1, 2);
    grid->addWidget(conflictBox,      2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->conflictButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this, &FileSaveConflictBox::signalConflictButtonChanged);

#else

    connect(d->conflictButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &FileSaveConflictBox::signalConflictButtonChanged);

#endif
}

FileSaveConflictBox::~FileSaveConflictBox()
{
    delete d;
}

void FileSaveConflictBox::resetToDefault()
{
    setConflictRule(DIFFNAME);
}

FileSaveConflictBox::ConflictRule FileSaveConflictBox::conflictRule() const
{
    return ((ConflictRule)(d->conflictButtonGroup->checkedId()));
}

void FileSaveConflictBox::setConflictRule(ConflictRule r)
{
    QAbstractButton* const bt = d->conflictButtonGroup->button((int)r);

    if (bt)
    {
        bt->setChecked(true);
    }
}

void FileSaveConflictBox::readSettings(const KConfigGroup& group)
{
    setConflictRule((FileSaveConflictBox::ConflictRule)group.readEntry("Conflict",
                    (int)(FileSaveConflictBox::DIFFNAME)));
}

void FileSaveConflictBox::writeSettings(KConfigGroup& group)
{
    group.writeEntry("Conflict", (int)conflictRule());
}

} // namespace Digikam

#include "moc_filesaveconflictbox.cpp"
