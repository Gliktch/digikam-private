/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * SPDX-FileCopyrightText: 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QList>
#include <QUrl>

// Local includes

#include "kmlexport.h"
#include "dfileselector.h"
#include "dcolorselector.h"
#include "dinfointerface.h"
#include "geolocationedit.h"
#include "gpsitemmodel.h"

class QButtonGroup;
class QCheckBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpacerItem;
class QComboBox;
class QCloseEvent;
class QSpinBox;
class QPushButton;

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class KmlWidget : public QWidget
{
    Q_OBJECT

public:

    explicit KmlWidget(GeolocationEdit* const dlg,
                       GPSItemModel* const imageModel,
                       DInfoInterface* const iface);
    ~KmlWidget() override;

public:

    QLabel*           ImageSizeLabel                = nullptr;
    QLabel*           IconSizeLabel                 = nullptr;
    QLabel*           destinationDirectoryLabel_    = nullptr;
    QLabel*           FileNameLabel_                = nullptr;
    QLabel*           DestinationUrlLabel_          = nullptr;
    QLabel*           GPXFileLabel_                 = nullptr;
    QLabel*           timeZoneLabel_                = nullptr;
    QLabel*           GPXLineWidthLabel_            = nullptr;
    QLabel*           GPXColorLabel_                = nullptr;
    QLabel*           GPXAltitudeLabel_             = nullptr;
    QLabel*           GPXTracksOpacityLabel_        = nullptr;

    QGroupBox*        TargetPreferenceGroupBox      = nullptr;
    QGroupBox*        TargetTypeGroupBox            = nullptr;

    QButtonGroup*     buttonGroupTargetType         = nullptr;

    QRadioButton*     LocalTargetRadioButton_       = nullptr;
    QRadioButton*     GoogleMapTargetRadioButton_   = nullptr;

    QLineEdit*        DestinationUrl_               = nullptr;
    QLineEdit*        FileName_                     = nullptr;

    QCheckBox*        GPXTracksCheckBox_            = nullptr;

    QComboBox*        AltitudeCB_                   = nullptr;
    QComboBox*        timeZoneCB                    = nullptr;
    QComboBox*        GPXAltitudeCB_                = nullptr;

    DColorSelector*   GPXTrackColor_                = nullptr;

    DFileSelector*    DestinationDirectory_         = nullptr;
    DFileSelector*    GPXFileUrlRequester_          = nullptr;
    QPushButton*      m_geneBtn                     = nullptr;

    QSpinBox*         ImageSizeInput_               = nullptr;
    QSpinBox*         IconSizeInput_                = nullptr;
    QSpinBox*         GPXTracksOpacityInput_        = nullptr;
    QSpinBox*         GPXLineWidthInput_            = nullptr;

    GPSItemModel*     m_model                       = nullptr;
    GeolocationEdit*  m_dlg                         = nullptr;

Q_SIGNALS:

    void signalSetUIEnabled(const bool enabledState);
    void signalProgressSetup(const int maxProgress, const QString& progressText);

public Q_SLOTS:

    void slotGoogleMapTargetRadioButtonToggled(bool);
    void slotKMLTracksCheckButtonToggled(bool);

protected:

    void saveSettings();
    void readSettings();

protected Q_SLOTS:

    void slotKMLGenerate();

protected:

    QGridLayout* KMLExportConfigLayout              = nullptr;
    QGridLayout* SizeGroupBoxLayout                 = nullptr;
    QGridLayout* TargetPreferenceGroupBoxLayout     = nullptr;
    QGridLayout* buttonGroupTargetTypeLayout        = nullptr;

    KmlExport    m_kmlExport;
};

} // namespace DigikamGenericGeolocationEditPlugin
