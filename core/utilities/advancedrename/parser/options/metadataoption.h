/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide metadata information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "option.h"
#include "ruledialog.h"
#include "exiftoolparser.h"

class QLineEdit;

namespace Digikam
{

class MetadataPanel;

class MetadataOptionDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit MetadataOptionDialog(Rule* const parent, QWidget* const widget);
    ~MetadataOptionDialog() override = default;

public:

    MetadataPanel* metadataPanel     = nullptr;
    QLineEdit*     separatorLineEdit = nullptr;

private:

    /// @note disabled
    explicit MetadataOptionDialog(QWidget*)                      = delete;
    MetadataOptionDialog(const MetadataOptionDialog&)            = delete;
    MetadataOptionDialog& operator=(const MetadataOptionDialog&) = delete;
};

// --------------------------------------------------------

class MetadataOption : public Option
{
    Q_OBJECT

public:

    explicit MetadataOption(QWidget* const widget);
    ~MetadataOption()                                            override;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                override;

private:

    QString parseMetadata(const QString& token, ParseSettings& settings);

private:

    static QHash<QUrl, ExifToolParser::ExifToolData> m_exifToolMetadataCache;
    static QHash<QUrl, MetaEngine::MetaDataMap>      m_exifMetadataCache;
    static QHash<QUrl, MetaEngine::MetaDataMap>      m_iptcMetadataCache;
    static QHash<QUrl, MetaEngine::MetaDataMap>      m_xmpMetadataCache;

private:

    /// @note disabled
    explicit MetadataOption(QObject*)                = delete;
    MetadataOption(const MetadataOption&)            = delete;
    MetadataOption& operator=(const MetadataOption&) = delete;
};

} // namespace Digikam
