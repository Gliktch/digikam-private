/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to MediaWiki web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Nathan Damie <nathan dot damie at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Iliya Ivanov <ilko2002 at abv dot bg>
 * SPDX-FileCopyrightText: 2012-2016 by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawikiwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QProgressBar>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "wstoolutils.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dtextedit.h"

namespace DigikamGenericMediaWikiPlugin
{

class Q_DECL_HIDDEN MediaWikiWidget::Private
{
public:

    Private() = default;

public:

    QWidget*                                 fileBox            = nullptr;
    DTextEdit*                               titleEdit          = nullptr;
    DTextEdit*                               descEdit           = nullptr;
    QLineEdit*                               dateEdit           = nullptr;
    QLineEdit*                               longitudeEdit      = nullptr;
    QLineEdit*                               latitudeEdit       = nullptr;
    DTextEdit*                               categoryEdit       = nullptr;

    QLabel*                                  loginHeaderLbl     = nullptr;
    QLineEdit*                               nameEdit           = nullptr;
    QLineEdit*                               passwdEdit         = nullptr;
    QScrollArea*                             newWikiSv          = nullptr;
    QLineEdit*                               newWikiNameEdit    = nullptr;
    QLineEdit*                               newWikiUrlEdit     = nullptr;
    QComboBox*                               wikiSelect         = nullptr;

    QLineEdit*                               authorEdit         = nullptr;
    QLineEdit*                               sourceEdit         = nullptr;

    DTextEdit*                               genCatEdit         = nullptr;
    DTextEdit*                               genTxtEdit         = nullptr;
    DTextEdit*                               genComEdit         = nullptr;

    QLabel*                                  headerLbl          = nullptr;
    QLabel*                                  wikiNameDisplayLbl = nullptr;
    QLabel*                                  userNameDisplayLbl = nullptr;
    QPushButton*                             changeUserBtn      = nullptr;

    QCheckBox*                               resizeChB          = nullptr;
    QSpinBox*                                dimensionSpB       = nullptr;
    QSpinBox*                                imageQualitySpB    = nullptr;
    QCheckBox*                               removeMetaChB      = nullptr;
    QCheckBox*                               removeGeoChB       = nullptr;
    QComboBox*                               licenseComboBox    = nullptr;

    QGroupBox*                               loginGBox          = nullptr;
    QGroupBox*                               userGBox           = nullptr;

    DProgressWdg*                            progressBar        = nullptr;
    DInfoInterface*                          iface              = nullptr;
    DItemsList*                              imgList            = nullptr;

    QStringList                              WikisHistory;
    QStringList                              UrlsHistory;

    QString                                  defaultMessage     = i18nc("@info", "Select an image");

    QMap <QString, QMap <QString, QString> > imagesDescInfo;
};

MediaWikiWidget::MediaWikiWidget(DInfoInterface* const iface, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("MediaWikiWidget"));

    d->iface                      = iface;
    const int spacing             = layoutSpacing();

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);

    // -------------------------------------------------------------------

    d->headerLbl           = new QLabel(this);
    d->headerLbl->setWhatsThis(i18nc("@info", "This is a clickable link to open the MediaWiki home page in a web browser."));
    d->headerLbl->setOpenExternalLinks(true);
    d->headerLbl->setFocusPolicy(Qt::NoFocus);

    d->imgList             = new DItemsList(this);
    d->imgList->setObjectName(QLatin1String("WebService ImagesList"));
    d->progressBar         = new DProgressWdg(this);

    d->imgList->appendControlButtonsWidget(d->progressBar);
    QBoxLayout* const blay = d->imgList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    blay->setStretchFactor(d->progressBar, 20);

    d->imgList->setAllowRAW(true);
    d->imgList->setIface(d->iface);
    d->imgList->loadImagesFromCurrentSelection();
    d->imgList->listView()->setWhatsThis(i18nc("@info", "This is the list of images to upload to the wiki."));
    d->progressBar->hide();

    // --------------------- Upload tab ----------------------------------

    QScrollArea* const wrapperScroll   = new QScrollArea(this);
    DVBox* const wrapperPan            = new DVBox(wrapperScroll->viewport());
    wrapperScroll->setWidget(wrapperPan);
    wrapperScroll->setWidgetResizable(true);
    wrapperScroll->setVisible(false);

    QWidget* const wrapper             = new QWidget(wrapperPan);
    QHBoxLayout* const wrapperLayout   = new QHBoxLayout(wrapper);

    QScrollArea* const upload          = new QScrollArea(wrapper);
    DVBox* const pan                   = new DVBox(upload->viewport());
    pan->setAutoFillBackground(true);

    upload->setWidget(pan);
    upload->setWidgetResizable(true);

    DVBox* const uploadBox             = new DVBox(pan);
    QWidget* const uploadPanel         = new QWidget(uploadBox);
    QVBoxLayout* const uploadBoxLayout = new QVBoxLayout(uploadPanel);

    d->fileBox = new QWidget(uploadBox);
    d->fileBox->setWhatsThis(i18nc("@info", "This is the login form to your account on the chosen wiki."));
    QGridLayout* const fileBoxLayout = new QGridLayout(d->fileBox);

    loadItemInfoFirstLoad();

    d->titleEdit    = new DTextEdit(d->defaultMessage, d->fileBox);
    d->titleEdit->setLinesVisible(1);
    d->dateEdit     = new QLineEdit(d->defaultMessage, d->fileBox);

    d->descEdit     = new DTextEdit(0, d->fileBox);
    d->descEdit->setPlainText(d->defaultMessage);
    d->descEdit->setTabChangesFocus(1);
    d->descEdit->setAcceptRichText(false);
    d->categoryEdit = new DTextEdit(0, d->fileBox);
    d->categoryEdit->setPlainText(d->defaultMessage);
    d->categoryEdit->setTabChangesFocus(1);
    d->categoryEdit->setAcceptRichText(false);

    d->latitudeEdit  = new QLineEdit(d->defaultMessage, d->fileBox);
    d->longitudeEdit = new QLineEdit(d->defaultMessage, d->fileBox);

    QLabel* const titleLabel     = new QLabel(d->fileBox);
    titleLabel->setText(i18nc("@label: properties", "Title:"));
    QLabel* const dateLabel      = new QLabel(d->fileBox);
    dateLabel->setText(i18nc("@label: properties", "Date:"));
    QLabel* const descLabel      = new QLabel(d->fileBox);
    descLabel->setText(i18nc("@label: properties", "Description:"));
    QLabel* const categoryLabel  = new QLabel(d->fileBox);
    categoryLabel->setText(i18nc("@label: properties", "Categories:"));
    QLabel* const latitudeLabel  = new QLabel(d->fileBox);
    latitudeLabel->setText(i18nc("@label: properties", "Latitude:"));
    QLabel* const longitudeLabel = new QLabel(d->fileBox);
    longitudeLabel->setText(i18nc("@label: properties", "Longitude:"));

    uploadBoxLayout->setSpacing(spacing);
    uploadBoxLayout->addWidget(d->fileBox, 0, Qt::AlignTop);

    fileBoxLayout->addWidget(titleLabel,       1, 0, 1, 1);
    fileBoxLayout->addWidget(dateLabel,        2, 0, 1, 1);
    fileBoxLayout->addWidget(descLabel,        3, 0, 1, 1);
    fileBoxLayout->addWidget(categoryLabel,    4, 0, 1, 1);
    fileBoxLayout->addWidget(latitudeLabel,    5, 0, 1, 1);
    fileBoxLayout->addWidget(longitudeLabel,   6, 0, 1, 1);
    fileBoxLayout->addWidget(d->titleEdit,     1, 1, 1, 3);
    fileBoxLayout->addWidget(d->dateEdit,      2, 1, 1, 3);
    fileBoxLayout->addWidget(d->descEdit,      3, 1, 1, 3);
    fileBoxLayout->addWidget(d->categoryEdit,  4, 1, 1, 3);
    fileBoxLayout->addWidget(d->latitudeEdit,  5, 1, 1, 3);
    fileBoxLayout->addWidget(d->longitudeEdit, 6, 1, 1, 3);

    // --------------------- Config tab ----------------------------------

    QScrollArea* const config = new QScrollArea(wrapper);
    DVBox* const panel2       = new DVBox(config->viewport());
    config->setWidget(panel2);
    config->setWidgetResizable(true);

    // --------------------- Account area ----------------------------------

    d->userGBox = new QGroupBox(panel2);
    d->userGBox->setTitle(i18nc("@option", "Account"));
    d->userGBox->setWhatsThis(i18nc("@info", "This is the login form to your MediaWiki account."));

    QGridLayout* const loginBoxLayout = new QGridLayout(d->userGBox);
    d->userGBox->setLayout(loginBoxLayout);

    d->wikiSelect                 = new QComboBox(d->userGBox);
    d->wikiSelect->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    QPushButton* const newWikiBtn = new QPushButton(QIcon::fromTheme(QLatin1String("list-add")),
                                                    i18nc("@option: button", "New"), d->userGBox);
    newWikiBtn->setToolTip(i18nc("@info", "Add a wiki to this list"));
    d->nameEdit                   = new QLineEdit(d->userGBox);
    d->passwdEdit                 = new QLineEdit(d->userGBox);
    d->passwdEdit->setEchoMode(QLineEdit::Password);

    d->wikiSelect->addItem(i18nc("@item", "Wikimedia Commons"),  QLatin1String("https://commons.wikimedia.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikimedia Meta"),     QLatin1String("https://meta.wikimedia.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikipedia"),          QLatin1String("https://en.wikipedia.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikibooks"),          QLatin1String("https://en.wikibooks.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikinews"),           QLatin1String("https://en.wikinews.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikiquote"),          QLatin1String("https://en.wikiquote.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikisource"),         QLatin1String("https://en.wikinews.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wiktionary"),         QLatin1String("https://en.wiktionary.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "MediaWiki"),          QLatin1String("https://www.MediaWiki.org/w/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikia Foto"),         QLatin1String("https://foto.wikia.com/api.php"));
    d->wikiSelect->addItem(i18nc("@item", "Wikia Uncyclopedia"), QLatin1String("https://uncyclopedia.wikia.com/api.php"));

    d->wikiSelect->setEditable(false);

    QLabel* const wikiLabel = new QLabel(d->userGBox);
    wikiLabel->setText(i18nc("@label", "Wiki:"));

    // --------------------- New wiki area ----------------------------------

    d->newWikiSv              = new QScrollArea(this);
    DVBox* const newWikiPanel = new DVBox(d->newWikiSv->viewport());
    d->newWikiSv->setWidget(newWikiPanel);
    d->newWikiSv->setWidgetResizable(true);
    d->newWikiSv->setVisible(false);

    QWidget* const newWikiBox        = new QWidget(newWikiPanel);
    newWikiBox->setWhatsThis(i18nc("@info", "These are options for adding a Wiki."));

    QGridLayout* const newWikiLayout = new QGridLayout(newWikiBox);

    QLabel* const newWikiNameLabel   = new QLabel(newWikiPanel);
    newWikiNameLabel->setText(i18nc("@label: name of wiki area", "Name:"));

    QLabel* const newWikiUrlLabel    = new QLabel(newWikiPanel);
    newWikiUrlLabel->setText(i18nc("@label", "API URL:"));

    d->newWikiNameEdit            = new QLineEdit(newWikiPanel);
    d->newWikiUrlEdit             = new QLineEdit(newWikiPanel);

    QPushButton* const addWikiBtn = new QPushButton(QIcon::fromTheme(QLatin1String("list-add")),
                                                    i18nc("@action: button", "Add"), newWikiPanel);
    addWikiBtn->setToolTip(i18nc("@info", "Add a new wiki"));

    newWikiLayout->addWidget(newWikiNameLabel,   0, 0, 1, 1);
    newWikiLayout->addWidget(d->newWikiNameEdit, 0, 1, 1, 1);
    newWikiLayout->addWidget(newWikiUrlLabel,    1, 0, 1, 1);
    newWikiLayout->addWidget(d->newWikiUrlEdit,  1, 1, 1, 1);
    newWikiLayout->addWidget(addWikiBtn,         2, 1, 1, 1);

    QLabel* const nameLabel     = new QLabel(d->userGBox);
    nameLabel->setText(i18nc("@label", "Login:" ));

    QLabel* const passwdLabel   = new QLabel(d->userGBox);
    passwdLabel->setText(i18nc("@label", "Password:"));

    QPushButton* const loginBtn = new QPushButton(d->userGBox);
    loginBtn->setAutoDefault(true);
    loginBtn->setDefault(true);
    loginBtn->setText(i18nc("@action", "&Log in"));

    loginBoxLayout->addWidget(wikiLabel,     0, 0, 1, 1);
    loginBoxLayout->addWidget(d->wikiSelect, 0, 1, 1, 1);
    loginBoxLayout->addWidget(newWikiBtn,    0, 2, 1, 1);
    loginBoxLayout->addWidget(d->newWikiSv,  1, 1, 3, 3);
    loginBoxLayout->addWidget(nameLabel,     4, 0, 1, 1);
    loginBoxLayout->addWidget(d->nameEdit,   4, 1, 1, 1);
    loginBoxLayout->addWidget(passwdLabel,   5, 0, 1, 1);
    loginBoxLayout->addWidget(d->passwdEdit, 5, 1, 1, 1);
    loginBoxLayout->addWidget(loginBtn,      6, 0, 1, 1);
    loginBoxLayout->setObjectName(QLatin1String("loginBoxLayout"));

    // --------------------- Login info area ----------------------------------

    d->loginGBox = new QGroupBox(panel2);
    d->loginGBox->setTitle(i18nc("@option", "Login Information"));

    QGridLayout* const linfoBoxLayout = new QGridLayout(d->loginGBox);
    d->loginGBox->setLayout(linfoBoxLayout);

    QLabel* const wikiNameLbl = new QLabel(d->loginGBox);
    wikiNameLbl->setText(i18nc("@label: name of the wiki the user is currently logged on", "Logged on: "));
    d->wikiNameDisplayLbl     = new QLabel(d->loginGBox);

    QLabel* const userNameLbl = new QLabel(d->loginGBox);
    userNameLbl->setText(i18nc("@label: username which is used to connect to the wiki", "Logged as: "));
    d->userNameDisplayLbl     = new QLabel(d->loginGBox);

    d->changeUserBtn          = new QPushButton(QIcon::fromTheme(QLatin1String("system-switch-user")),
                                                i18nc("@action: button", "Change Account"), d->loginGBox);
    d->changeUserBtn->setToolTip(i18nc("@info", "Logout and change the account used for transfer"));

    linfoBoxLayout->addWidget(wikiNameLbl,           0, 0, 1, 1);
    linfoBoxLayout->addWidget(d->wikiNameDisplayLbl, 0, 1, 1, 1);
    linfoBoxLayout->addWidget(userNameLbl,           1, 0, 1, 1);
    linfoBoxLayout->addWidget(d->userNameDisplayLbl, 1, 1, 1, 1);
    linfoBoxLayout->addWidget(d->changeUserBtn,      2, 0, 1, 2);
    d->loginGBox->hide();

    // --------------------- Info area ----------------------------------

    QGroupBox* const textGBox = new QGroupBox(panel2);
    textGBox->setTitle(i18nc("@option", "Information"));
    textGBox->setWhatsThis(i18nc("@info", "This is the login form to your account on the chosen wiki."));

    QGridLayout* const textBoxLayout = new QGridLayout(textGBox);
    textGBox->setLayout(textBoxLayout);

    QLabel* const authorLbl  = new QLabel(i18nc("@label", "Author:"), textGBox);
    d->authorEdit            = new QLineEdit(textGBox);

    QLabel* const sourceLbl  = new QLabel(i18nc("@label: source file", "Source:"), textGBox);
    d->sourceEdit            = new QLineEdit(textGBox);

    QLabel* const licenseLbl = new QLabel(i18nc("@label", "License:"), textGBox);
    d->licenseComboBox       = new QComboBox(textGBox);
    d->licenseComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    d->licenseComboBox->addItem(i18nc("@item", "Own work, Creative Commons Attribution-Share Alike 4.0"),
                                       QLatin1String("{{self|cc-by-sa-4.0}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Own work, multi-license with CC-BY-SA-3.0 and GFDL"),
                                       QLatin1String("{{self|cc-by-sa-3.0|GFDL|migration=redundant}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Own work, multi-license with CC-BY-SA-3.0 and older"),
                                       QLatin1String("{{self|cc-by-sa-3.0,2.5,2.0,1.0}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Own work, Creative Commons Attribution-Share Alike 3.0"),
                                       QLatin1String("{{self|cc-by-sa-3.0}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Own work, Creative Commons Attribution 3.0"),
                                       QLatin1String("{{self|cc-by-3.0}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Own work, release into public domain under the CC-Zero license"),
                                       QLatin1String("{{self|cc-zero}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Author died more than 100 years ago"),
                                       QLatin1String("{{PD-old}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Photo of a two-dimensional work whose author died more than 100 years ago"),
                                       QLatin1String("{{PD-art}}"));
    d->licenseComboBox->addItem(i18nc("@item", "First published in the United States before 1923"),
                                       QLatin1String("{{PD-US}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Work of a U.S. government agency"),
                                       QLatin1String("{{PD-USGov}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Simple typefaces, individual words or geometric shapes"),
                                       QLatin1String("{{PD-text}}"));
    d->licenseComboBox->addItem(i18nc("@item", "Logos with only simple typefaces, individual words or geometric shapes"),
                                       QLatin1String("{{PD-textlogo}}"));
    d->licenseComboBox->addItem(i18nc("@item", "No license specified (not recommended for public wiki sites)"),
                                       QLatin1String(""));

    QLabel* const genCatLbl = new QLabel(i18nc("@label", "Generic categories:"), textGBox);
    d->genCatEdit           = new DTextEdit(0, textGBox);
    d->genCatEdit->setTabChangesFocus(1);
    d->genCatEdit->setWhatsThis(i18nc("@info", "This is a place to enter categories that will be added to all the files."));
    d->genCatEdit->setAcceptRichText(false);

    QLabel* const genTxtLbl = new QLabel(i18nc("@label", "Generic text:"), textGBox);
    d->genTxtEdit           = new DTextEdit(0, textGBox);
    d->genTxtEdit->setTabChangesFocus(1);
    d->genTxtEdit->setWhatsThis(i18nc("@info", "This is a place to enter text that will be added to all the files, "
                                      "below the Information template."));
    d->genTxtEdit->setAcceptRichText(false);

    QLabel* const genComLbl = new QLabel(i18nc("@label", "Upload comments:"), textGBox);
    d->genComEdit           = new DTextEdit(0, textGBox);
    d->genComEdit->setTabChangesFocus(1);
    d->genComEdit->setWhatsThis(i18nc("@info", "This is a place to enter text that will be used as upload comments. "
                                      "The default of 'Uploaded via digiKam uploader' will be used if empty."));
    d->genComEdit->setAcceptRichText(false);

    textBoxLayout->addWidget(authorLbl,          1, 0, 1, 1);
    textBoxLayout->addWidget(sourceLbl,          2, 0, 1, 1);
    textBoxLayout->addWidget(licenseLbl,         3, 0, 1, 1);
    textBoxLayout->addWidget(genCatLbl,          4, 0, 1, 1);
    textBoxLayout->addWidget(genTxtLbl,          5, 0, 1, 1);
    textBoxLayout->addWidget(genComLbl,          6, 0, 1, 1);

    textBoxLayout->addWidget(d->authorEdit,      1, 2, 1, 2);
    textBoxLayout->addWidget(d->sourceEdit,      2, 2, 1, 2);
    textBoxLayout->addWidget(d->licenseComboBox, 3, 2, 1, 2);
    textBoxLayout->addWidget(d->genCatEdit,      4, 2, 1, 2);
    textBoxLayout->addWidget(d->genTxtEdit,      5, 2, 1, 2);
    textBoxLayout->addWidget(d->genComEdit,      6, 2, 1, 2);
    textBoxLayout->setObjectName(QLatin1String("textBoxLayout"));

    // --------------------- Options area ----------------------------------

    QGroupBox* const optGBox = new QGroupBox(panel2);
    optGBox->setTitle(i18nc("@option", "Options"));
    optGBox->setWhatsThis(i18nc("@info", "These are options that will be applied to photos before upload."));

    QGridLayout* const optionsBoxLayout = new QGridLayout(optGBox);
    optGBox->setLayout(optionsBoxLayout);

    d->resizeChB = new QCheckBox(optGBox);
    d->resizeChB->setText(i18nc("@option", "Resize photos before uploading"));
    d->resizeChB->setChecked(false);

    d->dimensionSpB            = new QSpinBox(optGBox);
    d->dimensionSpB->setMinimum(0);
    d->dimensionSpB->setMaximum(10000);
    d->dimensionSpB->setSingleStep(10);
    d->dimensionSpB->setValue(1600);
    d->dimensionSpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->dimensionSpB->setEnabled(false);
    QLabel* const dimensionLbl = new QLabel(i18nc("@label", "Maximum size:"), optGBox);

    d->imageQualitySpB         = new QSpinBox(optGBox);
    d->imageQualitySpB->setMinimum(0);
    d->imageQualitySpB->setMaximum(100);
    d->imageQualitySpB->setSingleStep(1);
    d->imageQualitySpB->setValue(85);
    d->imageQualitySpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const imageQualityLbl = new QLabel(i18nc("@label", "JPEG quality:"), optGBox);

    d->removeMetaChB = new QCheckBox(optGBox);
    d->removeMetaChB->setText(i18nc("@option", "Remove metadata from file"));
    d->removeMetaChB->setChecked(false);

    d->removeGeoChB = new QCheckBox(optGBox);
    d->removeGeoChB->setText(i18nc("@option", "Remove coordinates from file"));
    d->removeGeoChB->setChecked(false);

    optionsBoxLayout->addWidget(d->resizeChB,       0, 0, 1, 2);
    optionsBoxLayout->addWidget(dimensionLbl,       1, 0, 1, 1);
    optionsBoxLayout->addWidget(imageQualityLbl,    2, 0, 1, 1);
    optionsBoxLayout->addWidget(d->dimensionSpB,    1, 1, 1, 1);
    optionsBoxLayout->addWidget(d->imageQualitySpB, 2, 1, 1, 1);
    optionsBoxLayout->addWidget(d->removeMetaChB,   3, 0, 1, 2);
    optionsBoxLayout->addWidget(d->removeGeoChB,    4, 0, 1, 2);
    optionsBoxLayout->setRowStretch(3, 10);
    optionsBoxLayout->setSpacing(spacing);
    optionsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    // ------------------------------------------------------------------------

    QTabWidget* const tabWidget = new QTabWidget;
    tabWidget->addTab(upload, i18nc("@item", "Items Properties"));
    tabWidget->addTab(config, i18nc("@item", "Upload Settings"));
    tabWidget->setMinimumWidth(350);

    // ------------------------------------------------------------------------

    wrapperLayout->addWidget(d->imgList);
    wrapperLayout->addWidget(tabWidget);
    wrapperLayout->setStretch(0, 10);
    wrapperLayout->setStretch(1, 5);

    mainLayout->addWidget(d->headerLbl);
    mainLayout->addWidget(wrapper);
    mainLayout->setSpacing(spacing);
    mainLayout->setContentsMargins(QMargins());

    updateLabels();  // use empty labels until login

    // --------------------- Slots definition ----------------------------------

    connect(d->resizeChB, SIGNAL(clicked()),
            this, SLOT(slotResizeChecked()));

    connect(d->removeMetaChB, SIGNAL(clicked()),
            this, SLOT(slotRemoveMetaChecked()));

    connect(d->changeUserBtn, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(loginBtn, SIGNAL(clicked()),
            this, SLOT(slotLoginClicked()));

    connect(newWikiBtn, SIGNAL(clicked()),
            this, SLOT(slotNewWikiClicked()));

    connect(addWikiBtn, SIGNAL(clicked()),
            this, SLOT(slotAddWikiClicked()));

    connect(d->titleEdit, SIGNAL(editingFinished()),
            this, SLOT(slotRestoreExtension()));

    connect(d->titleEdit, SIGNAL(textChanged()),
            this, SLOT(slotApplyTitle()));

    connect(d->dateEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotApplyDate()));

    // Problem: textChanged() is also called when the text is changed by setText()
    // textEdited() would be better, but QTextEdit does not have this.
    // Solution for the moment: we do not setText() when the selection changes

    connect(d->categoryEdit, SIGNAL(textChanged()),
            this, SLOT(slotApplyCategories()));

    // Problem: textChanged() is also called when the text is changed by setText()
    // textEdited() would be better, but QTextEdit does not have this.
    // Solution for the moment: we do not setText() when the selection changes

    connect(d->descEdit, SIGNAL(textChanged()),
            this, SLOT(slotApplyDescription()));

    connect(d->latitudeEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotApplyLatitude()));

    connect(d->longitudeEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotApplyLongitude()));

    connect(d->imgList, SIGNAL(signalItemClicked(QTreeWidgetItem*)),
            this, SLOT(slotLoadImagesDesc(QTreeWidgetItem*)));

    connect(d->imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotRemoveImagesDesc()));
}

MediaWikiWidget::~MediaWikiWidget()
{
    delete d;
}

void MediaWikiWidget::readSettings(KConfigGroup& group)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) <<  "Read settings from" << group.name();

    d->authorEdit->setText(group.readEntry("Author",           ""));
    d->sourceEdit->setText(group.readEntry("Source",           "{{own}}"));

    d->genCatEdit->setText(group.readEntry("genCategories",    "Uploaded with digiKam uploader"));
    d->genTxtEdit->setText(group.readEntry("genText",          ""));

    d->genComEdit->setText(group.readEntry("Comments",         "Uploaded with digiKam uploader"));
    d->resizeChB->setChecked(group.readEntry("Resize",         false));
    d->dimensionSpB->setValue(group.readEntry("Dimension",     1600));
    d->imageQualitySpB->setValue(group.readEntry("Quality",    85));
    d->removeMetaChB->setChecked(group.readEntry("RemoveMeta", false));
    d->removeGeoChB->setChecked(group.readEntry("RemoveGeo",   false));
    slotResizeChecked();
    slotRemoveMetaChecked();

    d->WikisHistory = group.readEntry("Wikis history",         QStringList());
    d->UrlsHistory  = group.readEntry("Urls history",          QStringList());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UrlHistory.size: "
                                     << d->UrlsHistory.size()
                                     << "; WikisHistory.size:"
                                     << d->WikisHistory.size();

    for (int i = 0 ; i < d->UrlsHistory.size() && i < d->WikisHistory.size() ; ++i)
    {
        d->wikiSelect->addItem(d->WikisHistory.at(i), d->UrlsHistory.at(i));
    }
}

void MediaWikiWidget::saveSettings(KConfigGroup& group)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Save settings to" << group.name();

    group.writeEntry("Author",        d->authorEdit->text());
    group.writeEntry("Source",        d->sourceEdit->text());

    group.writeEntry("genCategories", d->genCatEdit->toPlainText());
    group.writeEntry("genText",       d->genTxtEdit->toPlainText());
    group.writeEntry("Comments",      d->genComEdit->toPlainText());

    group.writeEntry("Resize",        d->resizeChB->isChecked());
    group.writeEntry("Dimension",     d->dimensionSpB->value());
    group.writeEntry("Quality",       d->imageQualitySpB->value());

    group.writeEntry("RemoveMeta",    d->removeMetaChB->isChecked());
    group.writeEntry("RemoveGeo",     d->removeGeoChB->isChecked());
}

DItemsList* MediaWikiWidget::imagesList() const
{
    return d->imgList;
}

DProgressWdg* MediaWikiWidget::progressBar() const
{
    return d->progressBar;
}

void MediaWikiWidget::updateLabels(const QString& userName, const QString& wikiName, const QString& url)
{
    QString web = QLatin1String("https://www.MediaWiki.org");

    if (url.isEmpty())
    {
        d->wikiNameDisplayLbl->clear();
    }
    else
    {
        web = url;
        d->wikiNameDisplayLbl->setText(QString::fromLatin1("<b>%1</b>").arg(wikiName));
    }

    d->headerLbl->setText(QString::fromUtf8("<h2><b><a href='%1'><font color=\"#3B5998\">%2</font></a></b></h2>").arg(web).arg(wikiName));

    if (userName.isEmpty())
    {
        d->userNameDisplayLbl->clear();
    }
    else
    {
        d->userNameDisplayLbl->setText(QString::fromLatin1("<b>%1</b>").arg(userName));
    }
}

void MediaWikiWidget::invertAccountLoginBox()
{
    if (d->loginGBox->isHidden())
    {
        d->userGBox->hide();
        d->loginGBox->show();
    }
    else
    {
        d->userGBox->show();
        d->loginGBox->hide();
    }
}

void MediaWikiWidget::slotResizeChecked()
{
    d->dimensionSpB->setEnabled(d->resizeChB->isChecked());
    d->imageQualitySpB->setEnabled(d->resizeChB->isChecked());
}

void MediaWikiWidget::slotRemoveMetaChecked()
{
    d->removeGeoChB->setEnabled(!d->removeMetaChB->isChecked());
    d->removeGeoChB->setChecked(d->removeMetaChB->isChecked());
}

void MediaWikiWidget::slotChangeUserClicked()
{
    Q_EMIT signalChangeUserRequest();
}

void MediaWikiWidget::slotLoginClicked()
{
    Q_EMIT signalLoginRequest(d->nameEdit->text(), d->passwdEdit->text(),
                              d->wikiSelect->itemText(d->wikiSelect->currentIndex()),
                              d->wikiSelect->itemData(d->wikiSelect->currentIndex()).toUrl());
}

void MediaWikiWidget::slotNewWikiClicked()
{
    if (d->newWikiSv->isVisible())
    {
        d->newWikiSv->setVisible(false);
    }
    else
    {
        d->newWikiSv->setVisible(true);
    }
}

void MediaWikiWidget::slotAddWikiClicked()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("MediaWiki export settings"));

    d->UrlsHistory << d->newWikiUrlEdit->text();
    group.writeEntry(QLatin1String("Urls history"), d->UrlsHistory);

    d->WikisHistory << d->newWikiNameEdit->text();
    group.writeEntry(QLatin1String("Wikis history"), d->WikisHistory);

    d->wikiSelect->addItem(d->newWikiNameEdit->text(), d->newWikiUrlEdit->text());
    d->wikiSelect->setCurrentIndex(d->wikiSelect->count()-1);

    slotNewWikiClicked();
}

void MediaWikiWidget::loadItemInfoFirstLoad()
{
    QList<QUrl> urls = d->imgList->imageUrls(false);

    d->imagesDescInfo.clear();

    for (int j = 0 ; j < urls.size() ; ++j)
    {
        loadItemInfo(urls.at(j));
    }
}

void MediaWikiWidget::loadItemInfo(const QUrl& url)
{
    DItemInfo info(d->iface->itemInfo(url));
    QStringList keywar        = info.keywords();
    QString dt                = info.dateTime().toString(Qt::ISODate).replace(QLatin1Char('T'), QLatin1Char(' '), Qt::CaseSensitive);
    QString ttl               = info.name();
    QString desc              = info.title();
    QString currentCategories;
    QString lat;
    QString lng;

    for (int i = 0 ; i < keywar.size() ; ++i)
    {
        if (i == keywar.size() - 1)
        {
            currentCategories.append(keywar.at(i));
        }
        else
        {
            currentCategories.append(keywar.at(i)).append(QLatin1Char('\n'));
        }
    }

    if (info.hasGeolocationInfo())
    {
        lat = QString::number(info.latitude(),  'f', 9);
        lng = QString::number(info.longitude(), 'f', 9);
    }

    QMap<QString, QString> imageMetaData;
    imageMetaData[QLatin1String("title")]       = ttl;
    imageMetaData[QLatin1String("date")]        = dt;
    imageMetaData[QLatin1String("categories")]  = currentCategories;
    imageMetaData[QLatin1String("description")] = desc;
    imageMetaData[QLatin1String("latitude")]    = lat;
    imageMetaData[QLatin1String("longitude")]   = lng;
    d->imagesDescInfo.insert(url.toLocalFile(), imageMetaData);
}

void MediaWikiWidget::clearEditFields()
{
    d->titleEdit->setText(d->defaultMessage);
    d->dateEdit->setText(d->defaultMessage);
    d->descEdit->setText(d->defaultMessage);
    d->categoryEdit->setText(d->defaultMessage);
    d->latitudeEdit->setText(d->defaultMessage);
    d->longitudeEdit->setText(d->defaultMessage);
}

void MediaWikiWidget::slotLoadImagesDesc(QTreeWidgetItem* item)
{
    QList<QTreeWidgetItem*> selectedItems  = d->imgList->listView()->selectedItems();
    const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(item);

    if (!l_item)
    {
        return;
    }

    QMap<QString, QString> imageMetaData;

    if (!d->imagesDescInfo.contains(l_item->url().toLocalFile()))
    {
        loadItemInfo(l_item->url());
    }

    imageMetaData = d->imagesDescInfo[l_item->url().toLocalFile()];

    d->titleEdit->setText(imageMetaData[QLatin1String("title")]);
    d->dateEdit->setText(imageMetaData[QLatin1String("date")].replace(QLatin1Char('T'), QLatin1Char(' '), Qt::CaseSensitive));
    d->latitudeEdit->setText(imageMetaData[QLatin1String("latitude")]);
    d->longitudeEdit->setText(imageMetaData[QLatin1String("longitude")]);

    if (selectedItems.size() == 1)
    {
        d->categoryEdit->setText(imageMetaData[QLatin1String("categories")]);
        d->descEdit->setText(imageMetaData[QLatin1String("description")]);
    }
}

void MediaWikiWidget::slotRemoveImagesDesc()
{
    QList<QUrl> items = d->imgList->imageUrls();
    QStringList toRemove;

    for (QMap <QString, QMap <QString, QString> >::const_iterator it = d->imagesDescInfo.constBegin() ;
         it != d->imagesDescInfo.constEnd() ; ++it)
    {
        QString path = it.key();

        if (!items.contains(QUrl::fromLocalFile(path)))
        {
            toRemove << path;
        }
    }

    for (const QString& path : std::as_const(toRemove))
    {
        d->imagesDescInfo.remove(path);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Remove" << path << "; new length:" << d->imagesDescInfo.size();
    }
}

void MediaWikiWidget::slotRestoreExtension()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "RestoreExtension";

    QString imageTitle;
    QString originalExtension;
    QString currentExtension;
    QList<QUrl> urls;
    QMap<QString, QString> imageMetaData;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    // Build the list of items to rename

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        imageMetaData = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageTitle    = imageMetaData[QLatin1String("title")];

        // Add original extension if removed

        currentExtension  = imageTitle.split(QLatin1Char('.')).last();
        originalExtension = urls.at(i).toLocalFile().split(QLatin1Char('.')).last();

        if (QString::compare(currentExtension, originalExtension, Qt::CaseInsensitive) != 0)
        {
            imageTitle.append(QLatin1Char('.')).append(originalExtension);
            d->titleEdit->setText(imageTitle);
        }

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << urls.at(i).toLocalFile() << "renamed to" << imageTitle;
        imageMetaData[QLatin1String("title")]       = imageTitle;
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyTitle()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ApplyTitle";

    QString givenTitle = title();
    QString imageTitle;
    QString number;
    QList<QUrl> urls;
    QMap<QString, QString> imageMetaData;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();
    QStringList parts;

    const int minLength = givenTitle.count(QLatin1Char('#'));

    // Build the list of items to rename

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        imageMetaData = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageTitle    = givenTitle;

        // If there is at least one #, replace it the correct number

        if (minLength > 0)
        {
            parts      = imageTitle.split(QLatin1Char('#'), Qt::KeepEmptyParts);
            imageTitle = parts.first().append(QLatin1Char('#')).append(parts.last());
            number     = QString::number(i + 1);

            while (number.length() < minLength)
            {
                number.prepend(QLatin1Char('0'));
            }

            imageTitle.replace(imageTitle.indexOf(QLatin1Char('#')), 1, number);
        }

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << urls.at(i).toLocalFile() << "renamed to" << imageTitle;
        imageMetaData[QLatin1String("title")]       = imageTitle;
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyDate()
{
    QList<QUrl> urls;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData;
        imageMetaData                               = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("date")]        = date();
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyCategories()
{
    QList<QUrl> urls;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData;
        imageMetaData                               = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("categories")]  = categories();
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyDescription()
{
    QList<QUrl> urls;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData;
        imageMetaData                               = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("description")] = description();
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyLatitude()
{

    QList<QUrl> urls;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData;
        imageMetaData                               = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("latitude")]    = latitude();
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

void MediaWikiWidget::slotApplyLongitude()
{
    QList<QUrl> urls;
    QList<QTreeWidgetItem*> selectedItems = d->imgList->listView()->selectedItems();

    for (int i = 0 ; i < selectedItems.size() ; ++i)
    {
        const DItemsListViewItem* const l_item = dynamic_cast<DItemsListViewItem*>(selectedItems.at(i));

        if (l_item)
        {
            urls.append(l_item->url());
        }
    }

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData;
        imageMetaData                               = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("longitude")]   = longitude();
        d->imagesDescInfo[urls.at(i).toLocalFile()] = imageMetaData;
    }
}

QMap <QString,QMap <QString,QString> > MediaWikiWidget::allImagesDesc()
{
    QList<QUrl> urls = d->imgList->imageUrls(false);

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        QMap<QString, QString> imageMetaData          = d->imagesDescInfo[urls.at(i).toLocalFile()];
        imageMetaData[QLatin1String("author")]        = author();
        imageMetaData[QLatin1String("source")]        = source();
        imageMetaData[QLatin1String("license")]       = license();
        imageMetaData[QLatin1String("genCategories")] = genCategories();
        imageMetaData[QLatin1String("genText")]       = genText();
        imageMetaData[QLatin1String("comments")]      = genComments();
        d->imagesDescInfo[urls.at(i).toLocalFile()]   = imageMetaData;
    }

    return d->imagesDescInfo;
}

QString MediaWikiWidget::author() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::author()";
    return d->authorEdit->text();
}

QString MediaWikiWidget::source() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::source()";
    return d->sourceEdit->text();
}

QString MediaWikiWidget::genCategories() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::genCategories()";
    return d->genCatEdit->toPlainText();
}

QString MediaWikiWidget::genText() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::genText()";
    return d->genTxtEdit->toPlainText();
}

QString MediaWikiWidget::genComments() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::genComments()";
    return d->genComEdit->toPlainText();
}

int MediaWikiWidget::quality() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::quality()";
    return d->imageQualitySpB->value();
}

int MediaWikiWidget::dimension() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::dimension()";
    return d->dimensionSpB->value();
}

bool MediaWikiWidget::resize() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::resize()";
    return d->resizeChB->isChecked();
}

bool MediaWikiWidget::removeMeta() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::removeMeta()";
    return d->removeMetaChB->isChecked();
}

bool MediaWikiWidget::removeGeo() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::removeGeo()";
    return d->removeGeoChB->isChecked();
}

QString MediaWikiWidget::license() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::license()";
    return d->licenseComboBox->itemData(d->licenseComboBox->currentIndex()).toString();
}

QString MediaWikiWidget::title() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::title()";
    return d->titleEdit->text();
}

QString MediaWikiWidget::categories() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::categories()";
    return d->categoryEdit->toPlainText();
}

QString MediaWikiWidget::description() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::description()";
    return d->descEdit->toPlainText();
}

QString MediaWikiWidget::date() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::date()";
    return d->dateEdit->text();
}

QString MediaWikiWidget::latitude() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::latitude()";
    return d->latitudeEdit->text();
}

QString MediaWikiWidget::longitude() const
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MediaWikiWidget::longitude()";
    return d->longitudeEdit->text();
}

} // namespace DigikamGenericMediaWikiPlugin

#include "moc_mediawikiwidget.cpp"
