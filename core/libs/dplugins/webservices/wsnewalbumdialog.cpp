/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-01
 * Description : new album creation dialog for remote web service.
 *
 * SPDX-FileCopyrightText: 2010 by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wsnewalbumdialog.h"

// Qt includes

#include <QHBoxLayout>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN WSNewAlbumDialog::Private
{
public:

    explicit Private(QWidget* const widget, const QString& name)
      : toolName  (name),
        mainWidget(widget)
    {
        titleEdt       = new DTextEdit(1, nullptr);
        descEdt        = new DTextEdit(0, nullptr);
        locEdt         = new DTextEdit;
        dtEdt          = new QDateTimeEdit(QDateTime::currentDateTime());

        mainLayout     = new QVBoxLayout(mainWidget);

        albumBox       = new QGroupBox(i18nc("@title: album properties", "Album"), mainWidget);
        albumBoxLayout = new QGridLayout(albumBox);

        titleLabel     = new QLabel(i18nc("@label: album properties", "Title: "),       albumBox);
        dateLabel      = new QLabel(i18nc("@label: album properties", "Time Stamp: "),  albumBox);
        descLabel      = new QLabel(i18nc("@label: album properties", "Description: "), albumBox);
        locLabel       = new QLabel(i18nc("@label: album properties", "Location: "),    albumBox);

        buttonBox      = new QDialogButtonBox();
    }

    DTextEdit*         titleEdt         = nullptr;
    DTextEdit*         descEdt          = nullptr;
    DTextEdit*         locEdt           = nullptr;
    QDateTimeEdit*     dtEdt            = nullptr;

    QLabel*            titleLabel       = nullptr;
    QLabel*            dateLabel        = nullptr;
    QLabel*            descLabel        = nullptr;
    QLabel*            locLabel         = nullptr;

    QString            toolName;
    QDialogButtonBox*  buttonBox        = nullptr;

    QGridLayout*       albumBoxLayout   = nullptr;
    QGroupBox*         albumBox         = nullptr;

    QVBoxLayout*       mainLayout       = nullptr;
    QWidget*           mainWidget       = nullptr;
};

WSNewAlbumDialog::WSNewAlbumDialog(QWidget* const parent, const QString& toolName)
    : QDialog(parent),
      d      (new Private(this, toolName))
{
    d->mainWidget->setMinimumSize(500, 500);
    setWindowTitle(QString(d->toolName + QLatin1String(" New Album")));
    setModal(false);

    const int spacing = layoutSpacing();


    d->buttonBox->addButton(QDialogButtonBox::Ok);
    d->buttonBox->addButton(QDialogButtonBox::Cancel);
    d->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(d->titleEdt, SIGNAL(textChanged()),
            this, SLOT(slotTextChanged()));

    connect(d->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(d->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    d->albumBox->setLayout(d->albumBoxLayout);
    d->albumBox->setWhatsThis(i18nc("@info", "These are basic settings for the new %1 album.",d->toolName));

    d->titleEdt->setToolTip(i18nc("@info", "Title of the album that will be created (required)."));

    d->dtEdt->setDisplayFormat(QLatin1String("dd.MM.yyyy HH:mm"));
    d->dtEdt->setWhatsThis(i18nc("@info", "Date and Time of the album that will be created (optional)."));

    d->descEdt->setToolTip(i18nc("@info", "Description of the album that will be created (optional)."));

    d->locEdt->setToolTip(i18nc("@info", "Location of the album that will be created (optional)."));

    d->albumBoxLayout->addWidget(d->titleLabel, 0, 0);
    d->albumBoxLayout->addWidget(d->titleEdt,   0, 1);
    d->albumBoxLayout->addWidget(d->dateLabel,  1, 0);
    d->albumBoxLayout->addWidget(d->dtEdt,      1, 1);
    d->albumBoxLayout->addWidget(d->descLabel,  2, 0);
    d->albumBoxLayout->addWidget(d->descEdt,    2, 1);
    d->albumBoxLayout->addWidget(d->locLabel,   3, 0);
    d->albumBoxLayout->addWidget(d->locEdt,     3, 1);
    d->albumBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->albumBoxLayout->setSpacing(spacing);

    d->mainLayout->addWidget(d->albumBox);
    d->mainLayout->addWidget(d->buttonBox);
    d->mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->mainLayout->setSpacing(spacing);
    setLayout(d->mainLayout);
}

WSNewAlbumDialog::~WSNewAlbumDialog()
{
    delete d;
}

void WSNewAlbumDialog::slotTextChanged()
{
    if (QString::compare(getTitleEdit()->text(), QLatin1String(""), Qt::CaseInsensitive) == 0)
    {
        d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void WSNewAlbumDialog::hideDateTime()
{
    d->dtEdt->hide();
    d->dateLabel->hide();
    d->albumBoxLayout->removeWidget(d->dtEdt);
    d->albumBoxLayout->removeWidget(d->dateLabel);
}

void WSNewAlbumDialog::hideDesc()
{
    d->descEdt->hide();
    d->descLabel->hide();
    d->albumBoxLayout->removeWidget(d->descEdt);
    d->albumBoxLayout->removeWidget(d->descLabel);
}

void WSNewAlbumDialog::hideLocation()
{
    d->locEdt->hide();
    d->locLabel->hide();
    d->albumBoxLayout->removeWidget(d->locEdt);
    d->albumBoxLayout->removeWidget(d->locLabel);
}

QWidget* WSNewAlbumDialog::getMainWidget() const
{
    return d->mainWidget;
}

QGroupBox* WSNewAlbumDialog::getAlbumBox() const
{
    return d->albumBox;
}

DTextEdit* WSNewAlbumDialog::getTitleEdit() const
{
    return d->titleEdt;
}

DTextEdit* WSNewAlbumDialog::getDescEdit() const
{
    return d->descEdt;
}

DTextEdit* WSNewAlbumDialog::getLocEdit() const
{
    return d->locEdt;
}

QDateTimeEdit* WSNewAlbumDialog::getDateTimeEdit() const
{
    return d->dtEdt;
}

void WSNewAlbumDialog::addToMainLayout(QWidget* const widget)
{
    d->mainLayout->addWidget(widget);
    d->mainLayout->removeWidget(d->buttonBox);
    d->mainLayout->addWidget(d->buttonBox);
}
void WSNewAlbumDialog::setAlbumSuggestion(const QString& title)
{
    d->titleEdt->setText(title);
    d->titleEdt->selectAll();
}

void WSNewAlbumDialog::getBaseAlbumProperties(WSAlbum& baseAlbum)
{
    baseAlbum.title       = getTitleEdit()->text();
    baseAlbum.location    = getLocEdit()->text();
    baseAlbum.description = getDescEdit()->toPlainText();
}

QDialogButtonBox* WSNewAlbumDialog::getButtonBox() const
{
    return d->buttonBox;
}

} // namespace Digikam

#include "moc_wsnewalbumdialog.cpp"
