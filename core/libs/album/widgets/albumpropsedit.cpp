/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumpropsedit.h"

// Qt includes

#include <QCheckBox>
#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "coredb.h"
#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "dexpanderbox.h"
#include "ddatepicker.h"
#include "dtextedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumDatePicker : public DDatePicker
{
    Q_OBJECT

public:

    explicit AlbumDatePicker(QWidget* const widget)
        : DDatePicker(widget)
    {
    }

    ~AlbumDatePicker() override = default;

    void dateLineEnterPressed()
    {
        lineEnterPressed();
    }
};

// --------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumPropsEdit::Private
{

public:

    Private() = default;

public:

    QDialogButtonBox* buttons       = nullptr;

    QLabel*           topLabel      = nullptr;
    QComboBox*        categoryCombo = nullptr;
    QComboBox*        parentCombo   = nullptr;
    DTextEdit*        titleEdit     = nullptr;
    DPlainTextEdit*   commentsEdit  = nullptr;

    AlbumDatePicker*  datePicker    = nullptr;

    PAlbum*           album         = nullptr;
};

AlbumPropsEdit::AlbumPropsEdit(PAlbum* const album, bool create)
    : QDialog(nullptr),
      d      (new Private)
{
    setWindowTitle(create ? i18nc("@title:window, album properties", "New Album")
                          : i18nc("@title:window, album properties", "Edit Album"));

    d->buttons          = new QDialogButtonBox(QDialogButtonBox::Help |
                                               QDialogButtonBox::Ok   |
                                               QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->album            = album;
    QWidget* const page = new QWidget(this);
    QLabel* const logo  = new QLabel(page);

    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    d->topLabel         = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    if (create)
    {
        slotNewAlbumTextChanged(0);
    }
    else
    {
        d->topLabel->setText(i18nc("@label: album properties", "\"%1\"\nAlbum Properties", album->title()));
    }

    // --------------------------------------------------------

    DLineWidget* const topLine        = new DLineWidget(Qt::Horizontal);

    QLabel* const titleLabel          = new QLabel(page);
    titleLabel->setText(i18nc("@label: album properties", "&Title:"));

    d->titleEdit                      = new DTextEdit(page);
    d->titleEdit->setLinesVisible(1);
    d->titleEdit->setPlaceholderText(i18nc("@label: album properties", "Set here the new album title"));
    d->titleEdit->setIgnoredCharacters(QLatin1String("/:"));
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const categoryLabel       = new QLabel(page);
    categoryLabel->setText(i18nc("@label: album properties", "Ca&tegory:"));

    d->categoryCombo                  = new QComboBox(page);
    d->categoryCombo->setEditable(true);
    d->categoryCombo->lineEdit()->setClearButtonEnabled(true);
    categoryLabel->setBuddy(d->categoryCombo);

    QLabel* const parentLabel         = new QLabel(page);
    parentLabel->setText(i18nc("@label: album properties", "Ch&ild Of:"));

    d->parentCombo                    = new QComboBox(page);
    parentLabel->setBuddy(d->parentCombo);

    QLabel* const commentsLabel       = new QLabel(page);
    commentsLabel->setText(i18nc("@label: album properties", "Ca&ption:"));

    d->commentsEdit                   = new DPlainTextEdit(page);
    commentsLabel->setBuddy(d->commentsEdit);
    d->commentsEdit->setWordWrapMode(QTextOption::WordWrap);
    d->commentsEdit->setPlaceholderText(i18nc("@label: album properties", "Enter album caption here..."));

    QLabel* const dateLabel           = new QLabel(page);
    dateLabel->setText(i18nc("@label: album properties", "Album &date:"));

    d->datePicker                     = new AlbumDatePicker(page);
    dateLabel->setBuddy(d->datePicker);

    DHBox* const buttonRow            = new DHBox(page);
    QPushButton* const dateLowButton  = new QPushButton(i18nc("@action: Selects the date of the oldest image", "&Oldest"),  buttonRow);
    dateLowButton->setWhatsThis(i18nc("@info", "Use this button to select the date of the oldest image from album."));
    QPushButton* const dateAvgButton  = new QPushButton(i18nc("@action: Calculates the average date",          "&Average"), buttonRow);
    dateAvgButton->setWhatsThis(i18nc("@info", "Use this button to calculate the average date of images from album."));
    QPushButton* const dateHighButton = new QPushButton(i18nc("@action: Selects the date of the newest image", "Newest"),   buttonRow);
    dateHighButton->setWhatsThis(i18nc("@info", "Use this button to select the date of the newest image from album."));

    if (create)
    {
        setTabOrder(d->titleEdit,     d->categoryCombo);
        setTabOrder(d->categoryCombo, d->parentCombo);
        setTabOrder(d->parentCombo,   d->commentsEdit);
        setTabOrder(d->commentsEdit,  d->datePicker);
        dateHighButton->hide();
        dateAvgButton->hide();
        dateLowButton->hide();
    }
    else
    {
        setTabOrder(d->titleEdit,     d->categoryCombo);
        setTabOrder(d->categoryCombo, d->commentsEdit);
        setTabOrder(d->commentsEdit,  d->datePicker);
        d->parentCombo->hide();
        parentLabel->hide();
    }

    d->commentsEdit->setTabChangesFocus(true);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
    grid->addWidget(logo,             0, 0, 1, 1);
    grid->addWidget(d->topLabel,      0, 1, 1, 1);
    grid->addWidget(topLine,          1, 0, 1, 2);
    grid->addWidget(titleLabel,       2, 0, 1, 1);
    grid->addWidget(d->titleEdit,     2, 1, 1, 1);
    grid->addWidget(categoryLabel,    3, 0, 1, 1);
    grid->addWidget(d->categoryCombo, 3, 1, 1, 1);

    if (create)
    {
        grid->addWidget(parentLabel,      4, 0, 1, 1);
        grid->addWidget(d->parentCombo,   4, 1, 1, 1);
        grid->addWidget(commentsLabel,    5, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->commentsEdit,  5, 1, 1, 1);
        grid->addWidget(dateLabel,        6, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->datePicker,    6, 1, 1, 1);
        grid->addWidget(buttonRow,        7, 1, 1, 1);
    }
    else
    {
        grid->addWidget(commentsLabel,    4, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->commentsEdit,  4, 1, 1, 1);
        grid->addWidget(dateLabel,        5, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(d->datePicker,    5, 1, 1, 1);
        grid->addWidget(buttonRow,        6, 1, 1, 1);
    }

    grid->setSpacing(layoutSpacing());

    grid->setContentsMargins(QMargins());
    page->setLayout(grid);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // Initialize ---------------------------------------------

    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        QStringList Categories = settings->getAlbumCategoryNames();
        d->categoryCombo->addItems(Categories);
        int categoryIndex      = Categories.indexOf(album->category());

        if (categoryIndex != -1)
        {
            d->categoryCombo->setCurrentIndex(categoryIndex);
        }
        else
        {
            d->categoryCombo->lineEdit()->clear();
        }
    }

    if (create)
    {
        d->titleEdit->setText(i18nc("@label: album properties", "New Album"));
        d->datePicker->setDate(QDate::currentDate());
        d->parentCombo->addItem(i18nc("@item: album properties", "Selected Album (Default)"));
        d->parentCombo->addItem(i18nc("@item: top level folder of album","Root of current collection"));
    }
    else
    {
        d->titleEdit->setText(album->title());
        d->commentsEdit->setPlainText(album->caption());
        d->datePicker->setDate(album->date());
    }

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged()),
            this, SLOT(slotTitleChanged()));

    connect(dateLowButton, SIGNAL(clicked()),
            this, SLOT(slotDateLowButtonClicked()));

    connect(dateAvgButton, SIGNAL(clicked()),
            this, SLOT(slotDateAverageButtonClicked()));

    connect(dateHighButton, SIGNAL(clicked()),
            this, SLOT(slotDateHighButtonClicked()));

    connect(d->parentCombo, SIGNAL(activated(int)),
            this, SLOT(slotNewAlbumTextChanged(int)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // ---

    adjustSize();
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();
}

AlbumPropsEdit::~AlbumPropsEdit()
{
    delete d;
}

QString AlbumPropsEdit::title() const
{
    return d->titleEdit->text().trimmed();
}

QString AlbumPropsEdit::comments() const
{
    return d->commentsEdit->toPlainText();
}

QDate AlbumPropsEdit::date() const
{
    // See bug #267944 : update calendar view if user enter a date in text field.

    d->datePicker->dateLineEnterPressed();

    return d->datePicker->date();
}

int AlbumPropsEdit::parent() const
{
    return d->parentCombo->currentIndex();
}

QString AlbumPropsEdit::category() const
{
    QString name = d->categoryCombo->currentText();

    if (name.isEmpty())
    {
        name = i18nc("@info: album properties", "Uncategorized Album");
    }

    return name;
}

QStringList AlbumPropsEdit::albumCategories() const
{
    QStringList Categories;
    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (settings)
    {
        Categories = settings->getAlbumCategoryNames();
    }

    QString currentCategory = d->categoryCombo->currentText();

    if (!currentCategory.isEmpty())
    {
        if (Categories.indexOf(currentCategory) == -1)
        {
            Categories.append(currentCategory);
        }
    }

    Categories.sort();

    return Categories;
}

bool AlbumPropsEdit::editProps(PAlbum* const album, QString& title,
                               QString& comments, QDate& date, QString& category,
                               QStringList& albumCategories)
{
    QPointer<AlbumPropsEdit> dlg = new AlbumPropsEdit(album);

    bool ok = (dialogExec(dlg) == QDialog::Accepted);

    title           = dlg->title();
    comments        = dlg->comments();
    date            = dlg->date();
    category        = dlg->category();
    albumCategories = dlg->albumCategories();

    delete dlg;
    return ok;
}

bool AlbumPropsEdit::createNew(PAlbum* const parent, QString& title, QString& comments,
                               QDate& date, QString& category, QStringList& albumCategories,
                               int& parentSelector)
{
    QPointer<AlbumPropsEdit> dlg = new AlbumPropsEdit(parent, true);

    bool ok = (dialogExec(dlg) == QDialog::Accepted);

    title           = dlg->title();
    comments        = dlg->comments();
    date            = dlg->date();
    category        = dlg->category();
    albumCategories = dlg->albumCategories();
    parentSelector  = dlg->parent();

    delete dlg;
    return ok;
}

void AlbumPropsEdit::slotTitleChanged()
{
    QString newtitle = d->titleEdit->text();
    QRegularExpression emptyTitle(QRegularExpression::anchoredPattern(QLatin1String("^\\s*$")));
    bool enable      = (!emptyTitle.match(newtitle).hasMatch() && !newtitle.isEmpty());
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void AlbumPropsEdit::slotNewAlbumTextChanged(int index)
{
    QString atitle;

    if (index == 0)
    {
        atitle = d->album->title();
    }
    else
    {
        atitle = CollectionManager::instance()->albumRootLabel(d->album->albumRootId());
    }

    d->topLabel->setText(i18nc("@label: album properties", "Create new Album in\n\"%1\"", atitle));
}

void AlbumPropsEdit::slotDateLowButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate lowDate = CoreDbAccess().db()->getAlbumLowestDate(d->album->id());

    if (lowDate.isValid())
    {
        d->datePicker->setDate(lowDate);
    }

    setCursor(Qt::ArrowCursor);
}

void AlbumPropsEdit::slotDateHighButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate highDate = CoreDbAccess().db()->getAlbumHighestDate(d->album->id());

    if (highDate.isValid())
    {
        d->datePicker->setDate(highDate);
    }

    setCursor(Qt::ArrowCursor);
}

void AlbumPropsEdit::slotDateAverageButtonClicked()
{
    setCursor(Qt::WaitCursor);

    QDate avDate = CoreDbAccess().db()->getAlbumAverageDate(d->album->id());

    setCursor(Qt::ArrowCursor);

    if (avDate.isValid())
    {
        d->datePicker->setDate(avDate);
    }
    else
    {
        QMessageBox::critical(this, i18nc("@title:window, album properties", "Could not Calculate Average"),
                                    i18nc("@info: album properties", "Could not calculate date average for this album."));
    }
}

void AlbumPropsEdit::slotHelp()
{
    openOnlineDocumentation(QLatin1String("left_sidebar"),
                            QLatin1String("albums_view"),
                            QLatin1String("managing-albums"));
}

} // namespace Digikam

#include "albumpropsedit.moc"

#include "moc_albumpropsedit.cpp"
