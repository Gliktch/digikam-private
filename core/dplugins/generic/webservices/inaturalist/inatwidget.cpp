/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inatwidget_p.h"

namespace DigikamGenericINatPlugin
{

const DItemsListView::ColumnType ItemDate     = DItemsListView::ColumnType::User1;
const DItemsListView::ColumnType ItemLocation = DItemsListView::ColumnType::User2;

INatWidget::INatWidget(QWidget* const parent,
                       DInfoInterface* const iface,
                       const QString& serviceName)
    : WSSettingsWidget(parent, iface, serviceName),
      d               (new Private)
{
    d->serviceName = serviceName;

    // -- The account group. ----------------------------------------------

    // make room and insert a QLabel for an account icon

    QWidget* const userNameLbl                = getAccountBoxLayout()->itemAt(0)->widget();
    getAccountBoxLayout()->removeWidget(userNameLbl);
    getAccountBoxLayout()->addWidget(userNameLbl, 0, 0, 1, 1);
    d->accountIcon                            = new QLabel(getAccountBox());
    getAccountBoxLayout()->addWidget(d->accountIcon, 0, 1, 1, 1);

    // change user and remove account buttons go the in the row

    getAccountBoxLayout()->removeWidget(getChangeUserBtn());
    QHBoxLayout* const changeRemoveUserLayout = new QHBoxLayout();
    d->removeAccount                          = new QPushButton(getAccountBox());
    d->removeAccount->setText(i18n("Remove Account"));
    changeRemoveUserLayout->addWidget(getChangeUserBtn());
    changeRemoveUserLayout->addWidget(d->removeAccount);
    getAccountBoxLayout()->addLayout(changeRemoveUserLayout, 2, 0, 1, -1);

    // -- The image list --------------------------------------------------

    d->imglst = new DItemsList(this);


    d->imglst->setAllowRAW(true);
    d->imglst->setIface(iface);
    d->imglst->loadImagesFromCurrentSelection();

    d->imglst->listView()->setWhatsThis(i18n("This is the list of images to "
                                        "upload to an iNaturalist observation."));

    d->imglst->listView()->setColumn(ItemDate, i18n("Date"), true);
    d->imglst->listView()->setColumn(ItemLocation, i18n("Location"), true);

    // -- The observation group. ------------------------------------------

    QGroupBox* const idBox                  = new QGroupBox(i18n("Observation"), getSettingsBox());
    QVBoxLayout* const idBoxLayout          = new QVBoxLayout(idBox);

    d->identificationImage                  = new QLabel(idBox);
    d->identificationLabel                  = new QLabel(i18n("<i>no identification</i>"), idBox);
    d->identificationLabel->setWordWrap(true);
    QHBoxLayout* const identificationLayout = new QHBoxLayout();
    identificationLayout->addWidget(d->identificationImage);
    identificationLayout->addWidget(d->identificationLabel, 1);
    idBoxLayout->addLayout(identificationLayout);

    d->closestKnownObservation              = new QLabel(idBox);
    d->closestKnownObservation->setWordWrap(true);
    d->closestKnownObservation->setOpenExternalLinks(true);
    idBoxLayout->addWidget(d->closestKnownObservation);

    d->observationDescription               = new DPlainTextEdit(0, idBox);
    d->observationDescription->setPlaceholderText(i18n("Optionally describe "
                                                  "your observation or explain your identification here."));
    QTextDocument* const pdoc               = d->observationDescription->document();
    QFontMetrics fm(pdoc->defaultFont());
    QMargins margins                        = d->observationDescription->contentsMargins();
    const int nRows                         = 3;
    d->observationDescription->setFixedHeight(nRows * fm.lineSpacing() +
                                              2 * (pdoc->documentMargin() +
                                              d->observationDescription->frameWidth()) +
                                              margins.top() + margins.bottom());
    idBoxLayout->addWidget(d->observationDescription);
    d->observationDescription->hide();

    d->identificationEdit                   = new TaxonEdit(idBox);
    d->identificationEdit->setToolTip(i18n("Enter your identification here, "
                                           "popups will guide to a supported "
                                           "name."));
    d->identificationEdit->setPlaceholderText(i18n("Enter your identification "
                                                   "here; it is required."));
    d->taxonPopup                           = new SuggestTaxonCompletion(d->identificationEdit);
    idBoxLayout->addWidget(d->identificationEdit);

    QLabel* const placeLabel                = new QLabel(i18n("Place:"), idBox);
    d->placesComboBox                       = new QComboBox(idBox);
    d->placesComboBox->setInsertPolicy(QComboBox::NoInsert);
    d->placesComboBox->setEditable(true);
    d->moreOptionsButton                    = new QPushButton(i18n("More options"), idBox);
    d->moreOptionsButton->setCheckable(true);
    d->moreOptionsButton->setChecked(false);
    idBoxLayout->addWidget(d->moreOptionsButton);
    QHBoxLayout* const placesLayout         = new QHBoxLayout();
    placesLayout->addWidget(placeLabel);
    placesLayout->addWidget(d->placesComboBox, 1);
    placesLayout->addWidget(d->moreOptionsButton);
    idBoxLayout->addLayout(placesLayout);

    d->moreOptionsWidget                    = new QWidget(idBox);
    QVBoxLayout* const moreOptionsLayout    = new QVBoxLayout(d->moreOptionsWidget);

    QLabel* const warnTimeText1             = new QLabel(i18n("Photos should be taken within"), idBox);
    d->photoMaxTimeDiffSpB                  = new QSpinBox(idBox);
    d->photoMaxTimeDiffSpB->setMinimum(1);
    d->photoMaxTimeDiffSpB->setMaximum(9999);
    d->photoMaxTimeDiffSpB->setSingleStep(1);
    d->photoMaxTimeDiffSpB->setValue(5);
    d->photoMaxTimeDiffSpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->photoMaxTimeDiffSpB->setEnabled(true);
    QLabel* const warnTimeText2             = new QLabel(i18n("minute(s) of observation."), idBox);
    QHBoxLayout* const warnTimeLayout       = new QHBoxLayout();
    warnTimeLayout->addWidget(warnTimeText1);
    warnTimeLayout->addWidget(d->photoMaxTimeDiffSpB);
    warnTimeLayout->addWidget(warnTimeText2, 1);
    moreOptionsLayout->addLayout(warnTimeLayout);

    QLabel* const warnLocationText1         = new QLabel(i18n("Photos should be taken within"), idBox);
    d->photoMaxDistanceSpB                  = new QSpinBox(idBox);
    d->photoMaxDistanceSpB->setMinimum(1);
    d->photoMaxDistanceSpB->setMaximum(9999);
    d->photoMaxDistanceSpB->setSingleStep(1);
    d->photoMaxDistanceSpB->setValue(15);
    d->photoMaxDistanceSpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->photoMaxDistanceSpB->setEnabled(true);
    QLabel* const warnLocationText2         = new QLabel(i18n("meter(s) of observation."), idBox);
    QHBoxLayout* warnLocationLayout         = new QHBoxLayout();
    warnLocationLayout->addWidget(warnLocationText1);
    warnLocationLayout->addWidget(d->photoMaxDistanceSpB);
    warnLocationLayout->addWidget(warnLocationText2, 1);
    moreOptionsLayout->addLayout(warnLocationLayout);

    QLabel* const warnCloseObsText1         = new QLabel(i18n("Known observations should be within"), idBox);
    d->closestObservationMaxSpB             = new QSpinBox(idBox);
    d->closestObservationMaxSpB->setMinimum(10);
    d->closestObservationMaxSpB->setMaximum(99999);
    d->closestObservationMaxSpB->setSingleStep(10);
    d->closestObservationMaxSpB->setValue(500);
    d->closestObservationMaxSpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->closestObservationMaxSpB->setEnabled(true);
    QLabel* const warnCloseObsText2         = new QLabel(i18n("meter(s) of observation."), idBox);
    QHBoxLayout* const warnCloseObsLayout   = new QHBoxLayout();
    warnCloseObsLayout->addWidget(warnCloseObsText1);
    warnCloseObsLayout->addWidget(d->closestObservationMaxSpB);
    warnCloseObsLayout->addWidget(warnCloseObsText2, 1);
    moreOptionsLayout->addLayout(warnCloseObsLayout);
    idBoxLayout->addWidget(d->moreOptionsWidget);
    d->moreOptionsWidget->hide();

    idBox->show();
    getSettingsBoxLayout()->insertWidget(2, idBox);
    getSettingsBoxLayout()->setStretch(2, 5);

    // -- The options group. ------------------------------------------------

    getPhotoIdCheckBox()->show();

    // hiding widgets not needed here.

    getUploadBox()->hide();
    getSizeBox()->hide();
    getAlbumBox()->hide();

    replaceImageList(d->imglst);

    this->updateLabels(QString());
}

INatWidget::~INatWidget()
{
    delete d->taxonPopup;
    delete d;
}

void INatWidget::updateLabels(const QString& name, const QString& /*url*/)
{
    QString url = QLatin1String("https://www.inaturalist.org/");

    if (!name.isEmpty())
    {
        url += QLatin1String("observations?place_id=any&user_id=") + name +
               QLatin1String("&verifiable=any");
    }

    getHeaderLbl()->setText(i18n("<b><h2><a href=\"%1\"><font color=\"#74ac00\""
                                 ">iNaturalist</font></a> Export</h2></b>",
                                 url));
}

} // namespace DigikamGenericINatPlugin

#include "moc_inatwidget.cpp"
