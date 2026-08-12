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

#include "DgmlTextureTagWriter.h"

// Qt includes

#include <QUrl>

// Local includes

#include "GeoSceneTypes.h"
#include "GeoWriter.h"
#include "GeoSceneTileDataset.h"
#include "DownloadPolicy.h"
#include "DgmlElementDictionary.h"
#include "ServerLayout.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerTexture(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoSceneTypes::GeoSceneTileDatasetType), QString::fromUtf8(dgml::dgmlTag_nameSpace20)),
                                             new DgmlTextureTagWriter());

bool DgmlTextureTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoSceneTileDataset* texture = static_cast<const GeoSceneTileDataset*>(node);
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Texture));
    writer.writeAttribute(QString::fromUtf8("name"), texture->name());
    writer.writeAttribute(QString::fromUtf8("expire"), QString::number(texture->expire()));

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_SourceDir));
    writer.writeAttribute(QString::fromUtf8("format"), texture->fileFormat());

    if (texture->expire())
    {
        writer.writeAttribute(QString::fromUtf8("expire"), QString::number(texture->expire()));
    }

    writer.writeCharacters(texture->sourceDir());
    writer.writeEndElement();
    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_TileSize));
    writer.writeAttribute(QString::fromUtf8("width"), QString::number(texture->tileSize().width()));
    writer.writeAttribute(QString::fromUtf8("height"), QString::number(texture->tileSize().height()));
    writer.writeEndElement();

    writer.writeOptionalElement(QString::fromUtf8(dgml::dgmlTag_InstallMap), texture->installMap());

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_StorageLayout));

    if (texture->hasMaximumTileLevel())
    {
        writer.writeAttribute(QString::fromUtf8("maximumTileLevel"), QString::number(texture->maximumTileLevel()));
        writer.writeAttribute(QString::fromUtf8("levelZeroColumns"), QString::number(texture->levelZeroColumns()));
        writer.writeAttribute(QString::fromUtf8("levelZeroRows"), QString::number(texture->levelZeroRows()));
        writer.writeAttribute(QString::fromUtf8("mode"), texture->serverLayout()->name());
    }

    writer.writeEndElement();

    if (texture->downloadUrls().size() > 0)
    {
        for (int i = 0; i < texture->downloadUrls().size(); ++i)
        {
            QString protocol = texture->downloadUrls().at(i).toString().left(texture->downloadUrls().at(i).toString().indexOf(QLatin1Char(':')));
            QString host =  QString(texture->downloadUrls().at(i).host());
            int port =  texture->downloadUrls().at(i).port();
            QString path =  QString(texture->downloadUrls().at(i).path());
            QString query = texture->downloadUrls().at(i).query(QUrl::FullyEncoded);

            writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_DownloadUrl));
            writer.writeAttribute(QString::fromUtf8("protocol"), protocol);
            writer.writeAttribute(QString::fromUtf8("host"), host);

            if (port != -1)
            {
                writer.writeAttribute(QString::fromUtf8("port"), QString::number(port));
            }

            writer.writeAttribute(QString::fromUtf8("path"), path);
            writer.writeAttribute(QString::fromUtf8("query"), query);
            writer.writeEndElement();
        }
    }

    for (const DownloadPolicy* policy : texture->downloadPolicies())
    {
        writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_DownloadPolicy));

        if (policy->key().usage() == DownloadBrowse)
        {
            writer.writeAttribute(QString::fromUtf8("usage"), QString::fromUtf8("Browse"));
            writer.writeAttribute(QString::fromUtf8("maximumConnections"), QString::number(policy->maximumConnections()));
        }

        else if (policy->key().usage()  == DownloadBulk)
        {
            writer.writeAttribute(QString::fromUtf8("usage"), QString::fromUtf8("Bulk"));
            writer.writeAttribute(QString::fromUtf8("maximumConnections"), QString::number(policy->maximumConnections()));
        }

        writer.writeEndElement();
    }

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Projection));
    const GeoSceneAbstractTileProjection::Type tileProjectionType = texture->tileProjectionType();

    if (tileProjectionType == GeoSceneAbstractTileProjection::Mercator)
    {
        writer.writeAttribute(QString::fromUtf8("name"), QString::fromUtf8("Mercator"));
    }

    else if (tileProjectionType == GeoSceneAbstractTileProjection::Equirectangular)
    {
        writer.writeAttribute(QString::fromUtf8("name"), QString::fromUtf8("Equirectangular"));
    }

    writer.writeEndElement();

    writer.writeStartElement(QString::fromUtf8(dgml::dgmlTag_Blending));

    if (!texture->blending().isEmpty())
    {
        writer.writeAttribute(QString::fromUtf8("name"), texture->blending());
    }

    writer.writeEndElement();

    writer.writeEndElement();
    return true;
}

} // namespace Marble
