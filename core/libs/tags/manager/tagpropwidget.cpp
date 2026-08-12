/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-03
 * Description : Tag Properties widget to display tag properties
 *               when a tag or multiple tags are selected
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagpropwidget.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QPointer>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <kkeysequencewidget.h>
#include <kxmlgui_version.h>

#ifdef HAVE_KICONTHEMES
#   include <kicondialog.h>
#endif

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "searchtextbar.h"
#include "album.h"
#include "albummanager.h"
#include "tagsactionmngr.h"
#include "syncjob.h"
#include "dexpanderbox.h"
#include "dlayoutbox.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagPropWidget::Private
{
public:

    Private() = default;

public:

    QLabel*             topLabel        = nullptr;

    QString             icon;

    QPushButton*        iconButton      = nullptr;
    QPushButton*        resetIconButton = nullptr;
    QPushButton*        saveButton      = nullptr;
    QPushButton*        discardButton   = nullptr;
    QList<TAlbum*>      selectedAlbums;
    KKeySequenceWidget* keySeqWidget    = nullptr;
    DTextEdit*          titleEdit       = nullptr;
    bool                changed         = false;
};

TagPropWidget::TagPropWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing        = layoutSpacing();
    const int cmargin        = layoutMargin();
    QGridLayout* const grid  = new QGridLayout(this);
    QLabel* const logo       = new QLabel(this);

    logo->setPixmap(QIcon::fromTheme(QLatin1String("tag-properties")).pixmap(30,30));
    d->topLabel              = new QLabel(this);
    d->topLabel->setText(i18nc("@label", "Tag Properties"));
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    DLineWidget* const line  = new DLineWidget(Qt::Horizontal, this);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(this);
    titleLabel->setText(i18nc("@label: tag properties", "&Title:"));
    titleLabel->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    titleLabel->setIndent(spacing);

    d->titleEdit             = new DTextEdit(this);
    d->titleEdit->setPlaceholderText(i18nc("@info", "Enter tag name here"));
    d->titleEdit->setLinesVisible(1);
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const tipLabel   = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    tipLabel->setIndent(spacing);

    // ----------------------------------------------------------------------

    QLabel* const iconTextLabel = new QLabel(this);
    iconTextLabel->setText(i18nc("@label: tag properties", "&Icon:"));
    iconTextLabel->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    iconTextLabel->setIndent(spacing);

    d->iconButton               = new QPushButton(this);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    d->resetIconButton          = new QPushButton(QIcon::fromTheme(QLatin1String("view-refresh")),
                                                  i18nc("@action", "Reset"), this);

#ifndef HAVE_KICONTHEMES

    iconTextLabel->hide();
    d->iconButton->hide();
    d->resetIconButton->hide();

#endif

    // ----------------------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(this);
    kscTextLabel->setText(i18nc("@label: tag properties", "&Shortcut:"));
    kscTextLabel->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    kscTextLabel->setIndent(spacing);

    d->keySeqWidget            = new KKeySequenceWidget(this);
    kscTextLabel->setBuddy(d->keySeqWidget);

#if (KXMLGUI_VERSION >= QT_VERSION_CHECK(6, 12, 0))

    d->keySeqWidget->setPatterns(d->keySeqWidget->patterns() | KKeySequenceRecorder::Key);

#else

    d->keySeqWidget->setModifierlessAllowed(true);

#endif

    d->keySeqWidget->setCheckActionCollections(TagsActionMngr::defaultManager()->actionCollections());

    QLabel* const tipLabel2    = new QLabel(this);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(QString::fromUtf8("<p><b>%1:</b> <i>%2</i></p>")
                       .arg(i18nc("@label", "Note"))
                       .arg(i18nc("@label", "This shortcut can be used "
                                  "to assign or unassign tag to items.")));
    tipLabel2->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    tipLabel2->setIndent(spacing);

    d->saveButton              = new QPushButton(i18nc("@action", "Save"));
    d->discardButton           = new QPushButton(i18nc("@action", "Discard"));

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 1, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 0, 1, 4);
    grid->addWidget(tipLabel,           2, 0, 1, 4);
    grid->addWidget(titleLabel,         3, 0, 1, 1);
    grid->addWidget(d->titleEdit,       3, 1, 1, 3);
    grid->addWidget(iconTextLabel,      4, 0, 1, 1);
    grid->addWidget(d->iconButton,      4, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 2, 1, 1);
    grid->addWidget(kscTextLabel,       5, 0, 1, 1);
    grid->addWidget(d->keySeqWidget,    5, 1, 1, 3);
    grid->addWidget(tipLabel2,          6, 0, 1, 4);
    grid->addWidget(d->saveButton,      7, 0, 1, 1);
    grid->addWidget(d->discardButton,   7, 1, 1, 1);
    grid->setRowStretch(8, 10);
    grid->setColumnStretch(3, 10);
    grid->setContentsMargins(cmargin, cmargin, cmargin, cmargin);
    grid->setVerticalSpacing(spacing);

    adjustSize();

    // ---------------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->titleEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotDataChanged()));

    connect(d->titleEdit, SIGNAL(returnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->keySeqWidget, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(slotDataChanged()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveChanges()));

    connect(d->discardButton, SIGNAL(clicked()),
            this, SLOT(slotDiscardChanges()));

    enableItems(TagPropWidget::DisabledAll);
}

TagPropWidget::~TagPropWidget()
{
    delete d;
}

void TagPropWidget::slotSelectionChanged(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        enableItems(TagPropWidget::DisabledAll);
        return;
    }

    if (d->changed)
    {
        int rez = QMessageBox::question(this, qApp->applicationName(),
                                        i18nc("@info", "Previous tags were changed. "
                                              "Save changes? "));
        if (rez == QMessageBox::Yes)
        {
            slotSaveChanges();
        }

        d->changed = false;
    }

    if (albums.size() == 1)
    {
        TAlbum* const album = dynamic_cast<TAlbum*>(albums.first());

        if (!album)
        {
            return;
        }

        QString Seq = album->property(TagPropertyName::tagKeyboardShortcut());
        d->selectedAlbums.clear();
        d->selectedAlbums.append(album);
        d->titleEdit->setText(album->title());
        d->icon     = album->icon();
        d->iconButton->setIcon(SyncJob::getTagThumbnail(album));
        d->keySeqWidget->setKeySequence(Seq);

        if (album->isRoot())
        {
            enableItems(TagPropWidget::DisabledAll);
        }
        else
        {
            enableItems(TagPropWidget::EnabledAll);
        }
    }
    else
    {
        d->selectedAlbums.clear();
        bool containsRoot = false;
        QList<Album*>::const_iterator it;

        for (it = albums.constBegin() ; it != albums.constEnd() ; ++it)
        {
            TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

            if (temp)
            {
                d->selectedAlbums.append(temp);

                if (temp->isRoot())
                {
                    containsRoot = true;
                }
            }
        }

        d->titleEdit->clear();
        d->icon.clear();
        d->iconButton->setIcon(QIcon());
        d->keySeqWidget->clearKeySequence();

        if (containsRoot)
        {
            enableItems(TagPropWidget::DisabledAll);
        }
        else
        {
            enableItems(TagPropWidget::IconOnly);
        }
    }

    d->changed = false;
}

void TagPropWidget::slotFocusTitleEdit()
{
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();
}

void TagPropWidget::slotIconResetClicked()
{
    if (d->selectedAlbums.size() == 1)
    {
        const TAlbum* const tag = d->selectedAlbums.first();

        if (tag)
        {
            if (d->icon == tag->standardIconName())
            {
                return;
            }

            d->changed = true;
            d->icon    = tag->standardIconName();
            d->iconButton->setIcon(QIcon::fromTheme(d->icon));
        }
    }
}

void TagPropWidget::slotIconChanged()
{

#ifdef HAVE_KICONTHEMES

    QPointer<KIconDialog> dlg = new KIconDialog(this);
    dlg->setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon              = dlg->openDialog();
    delete dlg;

    if (icon.isEmpty() || (icon == d->icon))
    {
        return;
    }

    d->icon    = icon;
    d->changed = true;
    d->iconButton->setIcon(QIcon::fromTheme(d->icon));

#endif

}

void TagPropWidget::slotDataChanged()
{
    d->changed = true;
}

void TagPropWidget::slotSaveChanges()
{
    if (d->selectedAlbums.size() == 1)
    {
        QString title     = d->titleEdit->text();
        TAlbum* const tag = d->selectedAlbums.first();
        QString icon      = d->icon;
        QKeySequence ks   = d->keySeqWidget->keySequence();

        if (tag && (tag->title() != title))
        {
            QString errMsg;

            if (!AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }

        if (tag && (tag->icon() != icon))
        {
            QString errMsg;

            if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
            }
        }

        if (tag && (tag->property(TagPropertyName::tagKeyboardShortcut()) != ks.toString()))
        {
            TagsActionMngr::defaultManager()->updateTagShortcut(tag->id(), ks);
        }
    }
    else
    {
        QList<TAlbum*>::iterator it;

        for (it = d->selectedAlbums.begin() ; it != d->selectedAlbums.end() ; ++it)
        {
            TAlbum* const tag = *it;

            if (tag && (tag->icon() != d->icon))
            {
                QString errMsg;

                if (!AlbumManager::instance()->updateTAlbumIcon(tag, d->icon, 0, errMsg))
                {
                    QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), errMsg);
                }
            }
        }
    }

    d->changed = false;
}

void TagPropWidget::slotDiscardChanges()
{
    if (d->selectedAlbums.size() == 1)
    {
        TAlbum* const album = d->selectedAlbums.first();
        QString Seq         = album->property(TagPropertyName::tagKeyboardShortcut());

        d->titleEdit->setText(album->title());
        d->icon             = album->icon();
        d->iconButton->setIcon(SyncJob::getTagThumbnail(album));
        d->keySeqWidget->setKeySequence(Seq);
    }
    else
    {
        d->icon.clear();
    }

    d->changed = false;
}

void TagPropWidget::slotReturnPressed()
{
    slotSaveChanges();

    Q_EMIT signalTitleEditReady();
}

void TagPropWidget::enableItems(TagPropWidget::ItemsEnable value)
{
    bool val        = false;
    bool iconEnable = false;

    if (value == TagPropWidget::DisabledAll)
    {
        val        = false;
        iconEnable = false;
    }

    if (value == TagPropWidget::EnabledAll)
    {
        val        = true;
        iconEnable = true;
    }

    if (value == TagPropWidget::IconOnly)
    {
        val        = false;
        iconEnable = true;
    }

    d->titleEdit->setEnabled(val);
    d->keySeqWidget->setEnabled(val);
    d->resetIconButton->setEnabled(iconEnable);
    d->iconButton->setEnabled(iconEnable);
    d->saveButton->setEnabled(iconEnable);
    d->discardButton->setEnabled(iconEnable);
}

} // namespace Digikam

#include "moc_tagpropwidget.cpp"
