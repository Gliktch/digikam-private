/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : A widget to show details about images
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsitemdetails.h"

// C++ includes

#include <limits.h>
#include <math.h>

// Qt includes

#include <QCheckBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QIntValidator>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "graphicsdimgview.h"
#include "dimgpreviewitem.h"
#include "dexpanderbox.h"

namespace DigikamGenericGeolocationEditPlugin
{

class Q_DECL_HIDDEN GPSItemDetails::Private
{
public:

    Private() = default;

public:

    GPSItemModel*                imageModel             = nullptr;
    GraphicsDImgView*            previewManager         = nullptr;

    QCheckBox*                   cbCoordinates          = nullptr;
    QLineEdit*                   leLatitude             = nullptr;
    QLineEdit*                   leLongitude            = nullptr;
    QCheckBox*                   cbAltitude             = nullptr;
    QLineEdit*                   leAltitude             = nullptr;
    QCheckBox*                   cbSpeed                = nullptr;
    QLineEdit*                   leSpeed                = nullptr;
    QCheckBox*                   cbNSatellites          = nullptr;
    QLineEdit*                   leNSatellites          = nullptr;
    QCheckBox*                   cbFixType              = nullptr;
    QComboBox*                   comboFixType           = nullptr;
    QCheckBox*                   cbDop                  = nullptr;
    QLineEdit*                   leDop                  = nullptr;

    QPushButton*                 pbApply                = nullptr;

    QPersistentModelIndex        imageIndex;
    GPSDataContainer             infoOld;
    bool                         externalEnabledState   = true;
    bool                         activeState            = false;
    bool                         haveDelayedState       = false;
};

GPSItemDetails::GPSItemDetails(QWidget* const parent, GPSItemModel* const imageModel)
    : QWidget(parent),
      d      (new Private)
{
    d->imageModel = imageModel;

    // TODO: subscribe to changes in the model to update the display

    QVBoxLayout* const layout1 = new QVBoxLayout(this);

    // ----------------------------------

    QFormLayout* const formLayout = new QFormLayout();
    layout1->addLayout(formLayout);

    d->cbCoordinates = new QCheckBox(i18n("Coordinates"), this);
    formLayout->setWidget(formLayout->rowCount(), QFormLayout::LabelRole, d->cbCoordinates);

    d->leLatitude = new QLineEdit(this);
    d->leLatitude->setValidator(new QDoubleValidator(-90.0, 90.0, 12, this));
    d->leLatitude->setClearButtonEnabled(true);
    formLayout->addRow(i18n("Latitude"), d->leLatitude);
    d->leLongitude = new QLineEdit(this);
    d->leLongitude->setValidator(new QDoubleValidator(-180.0, 180.0, 12, this));
    d->leLongitude->setClearButtonEnabled(true);
    formLayout->addRow(i18n("Longitude"), d->leLongitude);

    d->cbAltitude = new QCheckBox(i18n("Altitude"), this);
    d->leAltitude = new QLineEdit(this);
    d->leAltitude->setClearButtonEnabled(true);
    d->leAltitude->setValidator(new QDoubleValidator(this));
    formLayout->addRow(d->cbAltitude, d->leAltitude);

    d->cbSpeed = new QCheckBox(i18n("Speed"), this);
    d->leSpeed = new QLineEdit(this);
    d->leSpeed->setClearButtonEnabled(true);
    d->leSpeed->setValidator(new QDoubleValidator(0, HUGE_VAL, 12, this));
    formLayout->addRow(d->cbSpeed, d->leSpeed);

    d->cbNSatellites = new QCheckBox(i18n("# satellites"), this);
    d->leNSatellites = new QLineEdit(this);
    d->leNSatellites->setClearButtonEnabled(true);
    d->leNSatellites->setValidator(new QIntValidator(0, 2000, this));
    formLayout->addRow(d->cbNSatellites, d->leNSatellites);

    d->cbFixType = new QCheckBox(i18n("Fix type"), this);
    d->comboFixType = new QComboBox(this);
    d->comboFixType->addItem(i18n("2-d"), QVariant(2));
    d->comboFixType->addItem(i18n("3-d"), QVariant(3));
    formLayout->addRow(d->cbFixType, d->comboFixType);

    d->cbDop = new QCheckBox(i18n("DOP"), this);
    d->leDop = new QLineEdit(this);
    d->leDop->setClearButtonEnabled(true);
    d->leDop->setValidator(new QDoubleValidator(0, HUGE_VAL, 2, this));
    formLayout->addRow(d->cbDop, d->leDop);

    d->pbApply = new QPushButton(i18n("Apply"), this);
    formLayout->setWidget(formLayout->rowCount(), QFormLayout::SpanningRole, d->pbApply);

    layout1->addWidget(new DLineWidget(Qt::Horizontal, this));

    // ----------------------------------

    d->previewManager = new GraphicsDImgView(this);
    d->previewManager->setItem(new DImgPreviewItem());
    d->previewManager->setMinimumSize(QSize(200, 200));
    layout1->addWidget(d->previewManager);

    // ----------------------------------

    connect(d->cbCoordinates, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->cbAltitude, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->cbSpeed, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->cbNSatellites, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->cbFixType, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->cbDop, SIGNAL(stateChanged(int)),
            this, SLOT(updateUIState()));

    connect(d->imageModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotModelDataChanged(QModelIndex,QModelIndex)));

    connect(d->pbApply, SIGNAL(clicked()),
            this, SLOT(slotApply()));

    updateUIState();
}

GPSItemDetails::~GPSItemDetails()
{
    delete d;
}

void GPSItemDetails::setUIEnabledExternal(const bool state)
{
    d->externalEnabledState = state;
    updateUIState();
}

void GPSItemDetails::saveSettingsToGroup(KConfigGroup* const /*group*/)
{
}

void GPSItemDetails::readSettingsFromGroup(const KConfigGroup* const /*group*/)
{
}

void GPSItemDetails::updateUIState()
{
    const bool externalEnabled = d->externalEnabledState && d->imageIndex.isValid();
    const bool haveCoordinates = d->cbCoordinates->isChecked();

    d->cbCoordinates->setEnabled(externalEnabled);

    d->leLatitude->setEnabled(haveCoordinates && externalEnabled);
    d->leLongitude->setEnabled(haveCoordinates && externalEnabled);

    /* altitude */

    d->cbAltitude->setEnabled(haveCoordinates && externalEnabled);
    const bool haveAltitude = d->cbAltitude->isChecked();
    d->leAltitude->setEnabled(haveAltitude && haveCoordinates && externalEnabled);

    /* speed */

    d->cbSpeed->setEnabled(haveCoordinates && externalEnabled);
    d->leSpeed->setEnabled(d->cbSpeed->isChecked() && haveCoordinates && externalEnabled);

    /* NSatellites */

    d->cbNSatellites->setEnabled(haveCoordinates && externalEnabled);
    d->leNSatellites->setEnabled(d->cbNSatellites->isChecked() && haveCoordinates && externalEnabled);

    /* fix type */

    d->cbFixType->setEnabled(haveCoordinates && externalEnabled);
    d->comboFixType->setEnabled(d->cbFixType->isChecked() && haveCoordinates && externalEnabled);

    /* dop */

    d->cbDop->setEnabled(haveCoordinates && externalEnabled);
    d->leDop->setEnabled(d->cbDop->isChecked() && haveCoordinates && externalEnabled);

    /* apply */

    d->pbApply->setEnabled(externalEnabled);
}

void GPSItemDetails::displayGPSDataContainer(const GPSDataContainer* const gpsData)
{
    d->cbAltitude->setChecked(false);
    d->cbSpeed->setChecked(false);
    d->leLatitude->clear();
    d->leLongitude->clear();
    d->leAltitude->clear();
    d->leSpeed->clear();
    d->leNSatellites->clear();
    d->leDop->clear();
    d->cbCoordinates->setChecked(gpsData->hasCoordinates());

    if (gpsData->hasCoordinates())
    {
        d->leLatitude->setText(QLocale().toString(gpsData->getCoordinates().lat(), 'f', 12));
        d->leLongitude->setText(QLocale().toString(gpsData->getCoordinates().lon(), 'f', 12));

        const bool haveAltitude = gpsData->hasAltitude();
        d->cbAltitude->setChecked(haveAltitude);

        if (haveAltitude)
        {
            d->leAltitude->setText(QLocale().toString(gpsData->getCoordinates().alt(), 'f', 12));
        }

        const bool haveSpeed = gpsData->hasSpeed();
        d->cbSpeed->setChecked(haveSpeed);

        if (haveSpeed)
        {
            d->leSpeed->setText(QLocale().toString(gpsData->getSpeed(), 'f', 12));
        }

        const bool haveNSatellites = gpsData->hasNSatellites();
        d->cbNSatellites->setChecked(haveNSatellites);

        if (haveNSatellites)
        {
            /// @todo Is this enough for simple integers or do we have to use QLocale?

            d->leNSatellites->setText(QString::number(gpsData->getNSatellites()));
        }

        const int haveFixType = gpsData->hasFixType();
        d->cbFixType->setChecked(haveFixType);

        if (haveFixType)
        {
            const int fixType      = gpsData->getFixType();
            const int fixTypeIndex = d->comboFixType->findData(QVariant(fixType));

            if (fixTypeIndex<0)
            {
                d->cbFixType->setChecked(false);
            }
            else
            {
                d->comboFixType->setCurrentIndex(fixTypeIndex);
            }
        }

        const bool haveDop = gpsData->hasDop();
        d->cbDop->setChecked(haveDop);

        if (haveDop)
        {
            d->leDop->setText(QString::fromLatin1("%1").arg(gpsData->getDop(), 0, 'f', 2));
        }
    }

    updateUIState();
}

void GPSItemDetails::slotSetCurrentImage(const QModelIndex& index)
{
    // TODO: slotSetActive may call this function with d->imageIndex as a parameter
    // since we get the index as a reference, we overwrite index when changing d->imageIndex

    QModelIndex indexCopy = index;
    d->imageIndex         = indexCopy;

    if (!d->activeState)
    {
        d->haveDelayedState = true;
        return;
    }

    d->haveDelayedState = false;

    GPSDataContainer gpsData;

    if (index.isValid())
    {
        const GPSItemContainer* const item = d->imageModel->itemFromIndex(index);
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << item;

        if (item)
        {
            d->previewManager->previewItem()->setPath(item->url().toLocalFile(), true);
            gpsData = item->gpsData();
        }
    }

    d->infoOld = gpsData;
    displayGPSDataContainer(&gpsData);
}

void GPSItemDetails::slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (!d->imageIndex.isValid())
    {
        return;
    }

    if (
        ((topLeft.row()    <= d->imageIndex.row()   ) && (bottomRight.row()    >= d->imageIndex.row())   ) &&
        ((topLeft.column() <= d->imageIndex.column()) && (bottomRight.column() >= d->imageIndex.column()))
       )
    {
        if (!d->activeState)
        {
            d->haveDelayedState = true;
            return;
        }

        GPSDataContainer gpsData;
        const GPSItemContainer* const item = d->imageModel->itemFromIndex(d->imageIndex);

        if (item)
        {
            d->previewManager->previewItem()->setPath(item->url().toLocalFile(), true);
            gpsData = item->gpsData();
        }

        d->infoOld = gpsData;
        displayGPSDataContainer(&gpsData);
    }
}

void GPSItemDetails::slotApply()
{
    GPSDataContainer newData;

    if (d->cbCoordinates->isChecked())
    {
        const double lat = QLocale().toDouble(d->leLatitude->text());
        const double lon = QLocale().toDouble(d->leLongitude->text());
        newData.setCoordinates(GeoCoordinates(lat, lon));

        if (d->cbAltitude->isChecked())
        {
            const qreal alt = static_cast<qreal>(QLocale().toDouble(d->leAltitude->text()));
            newData.setAltitude(alt);
        }

        if (d->cbSpeed->isChecked())
        {
            const qreal speed = static_cast<qreal>(QLocale().toDouble(d->leSpeed->text()));
            newData.setSpeed(speed);
        }

        if (d->cbNSatellites->isChecked())
        {
            const int nSatellites = d->leNSatellites->text().toInt();
            newData.setNSatellites(nSatellites);
        }

        if (d->cbFixType->isChecked())
        {
            const int fixType = d->comboFixType->itemData(d->comboFixType->currentIndex()).toInt();
            newData.setFixType(fixType);
        }

        if (d->cbDop->isChecked())
        {
            const qreal dop = static_cast<qreal>(QLocale().toDouble(d->leDop->text()));
            newData.setDop(dop);
        }
    }

    GPSItemContainer* const gpsItem   = d->imageModel->itemFromIndex(d->imageIndex);
    GPSUndoCommand* const undoCommand = new GPSUndoCommand();

    GPSUndoCommand::UndoInfo undoInfo(d->imageIndex);
    undoInfo.readOldDataFromItem(gpsItem);
    gpsItem->setGPSData(newData);
    undoInfo.readNewDataFromItem(gpsItem);
    undoCommand->addUndoInfo(undoInfo);
    undoCommand->setText(i18n("Details changed"));
    Q_EMIT signalUndoCommand(undoCommand);
}

void GPSItemDetails::slotSetActive(const bool state)
{
    d->activeState = state;

    if (state && d->haveDelayedState)
    {
        d->haveDelayedState = false;
        slotSetCurrentImage(d->imageIndex);
    }
}

} // namespace DigikamGenericGeolocationEditPlugin

#include "moc_gpsitemdetails.cpp"
