/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filters configuration dialog
 *
 * SPDX-FileCopyrightText: 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importfilterdlg.h"

// Qt includes

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPointer>
#include <QToolButton>
#include <QPainter>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>
#include <kmimetypechooser.h>
#include <kwidgetsaddons_version.h>

// Local includes

#include "digikam_debug.h"
#include "importfiltercombobox.h"
#include "dexpanderbox.h"
#include "dtextedit.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportFilterDlg::Private
{
public:

    Private() = default;

public:

    QDialogButtonBox*   buttons             = nullptr;

    DTextEdit*          filterName          = nullptr;
    QCheckBox*          mimeCheckBox        = nullptr;
    DAdjustableLabel*   mimeLabel           = nullptr;
    QToolButton*        mimeButton          = nullptr;
    QCheckBox*          fileNameCheckBox    = nullptr;
    QLineEdit*          fileNameEdit        = nullptr;
    QCheckBox*          pathCheckBox        = nullptr;
    QLineEdit*          pathEdit            = nullptr;
    QCheckBox*          newFilesCheckBox    = nullptr;
};

// ----------------------------------------------------------------------------------------

ImportFilterDlg::ImportFilterDlg(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "Edit Import Filters"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help |QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QWidget* const page               = new QWidget(this);
    QVBoxLayout* const verticalLayout = new QVBoxLayout(page);
    QLabel* label                     = nullptr;
    QHBoxLayout* horizontalLayout     = nullptr;
    QSpacerItem* spacer               = nullptr;

    // To prevent cppcheck warnings.
    (void)label;
    (void)horizontalLayout;
    (void)spacer;

    label            = new QLabel(page);
    label->setText(i18nc("@label: name of items filter", "Name:"));
    verticalLayout->addWidget(label);
    d->filterName    = new DTextEdit(page);
    d->filterName->setLinesVisible(1);
    verticalLayout->addWidget(d->filterName);

    d->mimeCheckBox  = new QCheckBox(page);
    d->mimeCheckBox->setText(i18n("Mime filter:"));
    verticalLayout->addWidget(d->mimeCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->mimeLabel     = new DAdjustableLabel(page);
    horizontalLayout->addWidget(d->mimeLabel);
    d->mimeButton    = new QToolButton(page);
    d->mimeButton->setText(i18n("Select Type Mime..."));
    horizontalLayout->addWidget(d->mimeButton);
    verticalLayout->addLayout(horizontalLayout);

    d->fileNameCheckBox = new QCheckBox(page);
    d->fileNameCheckBox->setText(i18n("File name filter:"));
    verticalLayout->addWidget(d->fileNameCheckBox);
    horizontalLayout    = new QHBoxLayout();
    spacer              = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->fileNameEdit     = new QLineEdit(page);
    horizontalLayout->addWidget(d->fileNameEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->pathCheckBox  = new QCheckBox(page);
    d->pathCheckBox->setText(i18n("Path filter:"));
    verticalLayout->addWidget(d->pathCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->pathEdit      = new QLineEdit(page);
    horizontalLayout->addWidget(d->pathEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->newFilesCheckBox = new QCheckBox(page);
    d->newFilesCheckBox->setText(i18n("Show only new files"));
    verticalLayout->addWidget(d->newFilesCheckBox);
    spacer              = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)),
            d->mimeButton, SLOT(setEnabled(bool)));

    connect(d->mimeButton, SIGNAL(clicked(bool)),
            this, SLOT(mimeButtonClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked(bool)),
            d->fileNameEdit, SLOT(setEnabled(bool)));

    connect(d->pathCheckBox, SIGNAL(clicked(bool)),
            d->pathEdit, SLOT(setEnabled(bool)));

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(mimeCheckBoxClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked()),
            this, SLOT(fileNameCheckBoxClicked()));

    connect(d->pathCheckBox, SIGNAL(clicked()),
            this, SLOT(pathCheckBoxClicked()));

    adjustSize();
}

ImportFilterDlg::~ImportFilterDlg()
{
    delete d;
}

void ImportFilterDlg::fileNameCheckBoxClicked()
{
    if (!d->fileNameCheckBox->isChecked())
    {
        d->fileNameEdit->clear();
    }
}

void ImportFilterDlg::pathCheckBoxClicked()
{
    if (!d->pathCheckBox->isChecked())
    {
        d->pathEdit->clear();
    }
}

void ImportFilterDlg::mimeCheckBoxClicked()
{
    if (!d->mimeCheckBox->isChecked())
    {
        d->mimeLabel->setAdjustedText();
    }
}

void ImportFilterDlg::mimeButtonClicked()
{
    QString text                         = i18n("Select the MimeTypes you want for this filter.");
    QStringList list                     = d->mimeLabel->adjustedText().split(QLatin1Char(';'), Qt::SkipEmptyParts);

    QPointer<KMimeTypeChooserDialog> dlg = new KMimeTypeChooserDialog(
                                                                      i18n("Select Mime Types"),
                                                                      text,
                                                                      list,
                                                                      QLatin1String("image"),
#if (KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(6, 24, 0))

                                                                      QStringList(),
                                                                      KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton,

#endif
                                                                      this
                                                                     );

    if (dlg->exec() == QDialog::Accepted)
    {
        d->mimeLabel->setAdjustedText(dlg->chooser()->mimeTypes().join(QLatin1Char(';')));
    }

    delete dlg;
}

void ImportFilterDlg::setData(const Filter& filter)
{
    d->filterName->setText(filter.name);
    d->mimeCheckBox->setChecked(!filter.mimeFilter.isEmpty());
    d->mimeLabel->setAdjustedText(filter.mimeFilter);
    d->mimeButton->setEnabled(!filter.mimeFilter.isEmpty());
    d->fileNameCheckBox->setChecked(!filter.fileFilter.isEmpty());
    d->fileNameEdit->setText(filter.fileFilter.join(QLatin1Char(';')));
    d->fileNameEdit->setEnabled(!filter.fileFilter.isEmpty());
    d->pathCheckBox->setChecked(!filter.pathFilter.isEmpty());
    d->pathEdit->setText(filter.pathFilter.join(QLatin1Char(';')));
    d->pathEdit->setEnabled(!filter.pathFilter.isEmpty());
    d->newFilesCheckBox->setChecked(filter.onlyNew);
}

void ImportFilterDlg::getData(Filter* const filter)
{
    filter->name       = d->filterName->text();
    filter->mimeFilter = d->mimeLabel->adjustedText();
    filter->fileFilter = d->fileNameEdit->text().split(QLatin1Char(';'), Qt::SkipEmptyParts);
    filter->pathFilter = d->pathEdit->text().split(QLatin1Char(';'), Qt::SkipEmptyParts);
    filter->onlyNew    = d->newFilesCheckBox->isChecked();
}

void ImportFilterDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"),
                            QLatin1String("camera_settings"),
                            QLatin1String("camera-filters"));
}

} // namespace Digikam

#include "moc_importfilterdlg.cpp"
