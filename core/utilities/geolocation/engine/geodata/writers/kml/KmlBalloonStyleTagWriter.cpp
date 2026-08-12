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

#include "KmlBalloonStyleTagWriter.h"

// Qt includes

#include <QRegularExpression>

// Local includes

#include "GeoDataBalloonStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlColorStyleTagWriter.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerBallonStyle(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataBalloonStyleType),
                                                                             QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                                 new KmlBalloonStyleTagWriter());

bool KmlBalloonStyleTagWriter::write(const GeoNode* node,
                                     GeoWriter& writer) const
{
    const GeoDataBalloonStyle* balloonStyle = static_cast<const GeoDataBalloonStyle*>(node);
    bool const isEmpty = balloonStyle->backgroundColor() == QColor(Qt::white) &&
                         balloonStyle->textColor() == QColor(Qt::black) &&
                         balloonStyle->text().isEmpty() &&
                         balloonStyle->displayMode() == GeoDataBalloonStyle::Default;

    if (isEmpty)
    {
        return true;
    }

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_BalloonStyle));
    KmlObjectTagWriter::writeIdentifiers(writer, balloonStyle);

    QString const backgroundColor = KmlColorStyleTagWriter::formatColor(balloonStyle->backgroundColor());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_bgColor), backgroundColor, QString::fromUtf8("ffffffff"));
    QString const textColor = KmlColorStyleTagWriter::formatColor(balloonStyle->textColor());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_textColor), textColor, QString::fromUtf8("ff000000"));

    QString const textString = balloonStyle->text();

    if (textString.contains(QRegularExpression(QString::fromUtf8("[<>&]"))))
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_text));
        writer.writeCDATA(textString);
        writer.writeEndElement();
    }

    else
    {
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_text), textString);
    }

    if (balloonStyle->displayMode() == GeoDataBalloonStyle::Hide)
    {
        writer.writeElement(QString::fromUtf8(kml::kmlTag_displayMode), QString::fromUtf8("hide"));
    }

    writer.writeEndElement();
    return true;
}

} // namespace Marble
