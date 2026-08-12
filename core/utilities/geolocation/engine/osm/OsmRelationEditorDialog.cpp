/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "OsmRelationEditorDialog.h"

// Qt includes

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "OsmTagEditorWidget.h"
#include "OsmPlacemarkData.h"
#include "OsmObjectManager.h"
#include "GeoDataPlacemark.h"
#include "GeoDataExtendedData.h"
#include "GeoDataData.h"

namespace Marble
{

OsmRelationEditorDialog::OsmRelationEditorDialog(OsmPlacemarkData* relationData, QWidget* parent)
    : QDialog(parent)
{
    m_relationData = relationData;
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Name input area
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel(i18n("Name"), this);
    m_nameLineEdit = new QLineEdit(this);
    m_nameLineEdit->setText(relationData->tagValue(QStringLiteral("name")));
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameLineEdit);
    layout->addLayout(nameLayout);

    // Tag editor area
    // A dummy placemark is needed because the OsmTagEditorWidget works with placemarks
    m_dummyPlacemark = new GeoDataPlacemark();
    // "osmRelaation=yes" entry is added to its ExtendedData to let the widget know
    // its special relation status
    GeoDataExtendedData extendedData;
    extendedData.addValue(GeoDataData(QStringLiteral("osmRelation"), QStringLiteral("yes")));
    m_dummyPlacemark->setExtendedData(extendedData);
    m_dummyPlacemark->setOsmData(*m_relationData);
    OsmObjectManager::initializeOsmData(m_dummyPlacemark);
    m_tagEditor = new OsmTagEditorWidget(m_dummyPlacemark, this);
    layout->addWidget(m_tagEditor);

    // Button box area
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(m_buttonBox);

    QObject::connect(m_buttonBox, SIGNAL(accepted()),
                     this, SLOT(checkFields()));
    connect(m_buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

OsmRelationEditorDialog::~OsmRelationEditorDialog()
{
    delete m_dummyPlacemark;
}

void OsmRelationEditorDialog::finish()
{
    // Updating the relation data with the edited one
    m_dummyPlacemark->osmData().addTag(QStringLiteral("name"), m_nameLineEdit->text());
    *m_relationData = m_dummyPlacemark->osmData();
    accept();
}

void OsmRelationEditorDialog::checkFields()
{
    if (m_nameLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,
                             i18n("No name specified"),
                             i18n("Please specify a name for this relation."));
    }

    else if (!m_dummyPlacemark->osmData().containsTagKey(QStringLiteral("type")))
    {
        QMessageBox::warning(this,
                             i18n("No type tag specified"),
                             i18n("Please add a type tag for this relation."));
    }

    else
    {
        finish();
    }
}

} // namespace Marble

#include "moc_OsmRelationEditorDialog.cpp"
