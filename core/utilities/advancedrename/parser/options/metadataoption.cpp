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

#include "metadataoption.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QScopedPointer>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dmetadata.h"
#include "metadatapanel.h"
#include "metadataselector.h"

namespace Digikam
{

MetadataOptionDialog::MetadataOptionDialog(Rule* const parent, QWidget* const widget)
    : RuleDialog(parent, widget)
{
    QWidget* const mainWidget = new QWidget(this);
    QTabWidget* const tab     = new QTabWidget(this);
    metadataPanel             = new MetadataPanel(tab);
    QLabel* const customLabel = new QLabel(i18n("Keyword separator:"));
    separatorLineEdit         = new QLineEdit(this);
    separatorLineEdit->setText(QLatin1String("_"));

    // --------------------------------------------------------

    // We only need the "SearchBar" control element.
    // We also need to reset the default selections.

    const auto list = metadataPanel->viewers();

    for (MetadataSelectorView* const viewer : list)
    {
        viewer->setControlElements(MetadataSelectorView::SearchBar);
        viewer->clearSelection();
    }

    // --------------------------------------------------------

    // remove "Viewer" string from tabs
    int tabs = tab->count();

    for (int i = 0 ; i < tabs ; ++i)
    {
        QString text = tab->tabText(i);
        text.remove(QLatin1String("viewer"), Qt::CaseInsensitive);
        tab->setTabText(i, text.simplified());
    }

    // --------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(this);
    mainLayout->addWidget(customLabel,       0, 0, 1, 1);
    mainLayout->addWidget(separatorLineEdit, 0, 1, 1, 1);
    mainLayout->addWidget(tab,               1, 0, 1, -1);
    mainWidget->setLayout(mainLayout);

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
    resize(450, 450);
}

// --------------------------------------------------------

QHash<QUrl, ExifToolParser::ExifToolData> MetadataOption::m_exifToolMetadataCache;
QHash<QUrl, MetaEngine::MetaDataMap>      MetadataOption::m_exifMetadataCache;
QHash<QUrl, MetaEngine::MetaDataMap>      MetadataOption::m_iptcMetadataCache;
QHash<QUrl, MetaEngine::MetaDataMap>      MetadataOption::m_xmpMetadataCache;

MetadataOption::MetadataOption(QWidget* const widget)
    : Option(i18n("Metadata..."),
             i18n("Add metadata information"))
{
    QString iconName(QLatin1String("format-text-code"));
    setIcon(iconName);

    // --------------------------------------------------------

    addToken(QLatin1String("[meta:||key||]"), description());

    QRegularExpression reg(QLatin1String("\\[meta(:(.*))\\]"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
    setParentWidget(widget);
}

MetadataOption::~MetadataOption()
{
    m_exifToolMetadataCache.clear();
    m_exifMetadataCache.clear();
    m_iptcMetadataCache.clear();
    m_xmpMetadataCache.clear();
}

void MetadataOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QStringList tags;

    QPointer<MetadataOptionDialog> dlg = new MetadataOptionDialog(this, getParentWidget());

    if (dialogExec(dlg) == QDialog::Accepted)
    {
        QStringList checkedTags = dlg->metadataPanel->getAllCheckedTags();

        for (const QString& tag : std::as_const(checkedTags))
        {
            tags << QString::fromUtf8("[meta:%1]").arg(tag);
        }
    }

    if (!tags.isEmpty())
    {
        QString tokenStr = tags.join(dlg->separatorLineEdit->text());

        Q_EMIT signalTokenTriggered(tokenStr);
    }

    delete dlg;
}

QString MetadataOption::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString keyword = match.captured(2);
    QString result  = parseMetadata(keyword, settings);

    return result;
}

QString MetadataOption::parseMetadata(const QString& token, ParseSettings& settings)
{
    QString result;

    if (settings.fileUrl.isEmpty())
    {
        return result;
    }

    QString keyword = token.toLower();

    if (keyword.isEmpty())
    {
        return result;
    }

    // It only needs to check m_exifMetadataCache to contain
    // the file url, all other caches also have the file url.

    if (!m_exifMetadataCache.contains(settings.fileUrl))
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        (void)meta->load(settings.fileUrl.toLocalFile(), true);

        m_exifMetadataCache.insert(settings.fileUrl,
                                   meta->getExifTagsDataList(QStringList(), true));
        m_iptcMetadataCache.insert(settings.fileUrl,
                                   meta->getIptcTagsDataList(QStringList(), true));
        m_xmpMetadataCache.insert(settings.fileUrl,
                                  meta->getXmpTagsDataList(QStringList(), true));
    }

    MetaEngine::MetaDataMap dataMap;

    if      (keyword.startsWith(QLatin1String("exif.")))
    {
        dataMap = m_exifMetadataCache.value(settings.fileUrl);
    }
    else if (keyword.startsWith(QLatin1String("iptc.")))
    {
        dataMap = m_iptcMetadataCache.value(settings.fileUrl);
    }
    else if (keyword.startsWith(QLatin1String("xmp.")))
    {
        dataMap = m_xmpMetadataCache.value(settings.fileUrl);
    }

    const auto keys = dataMap.keys();

    for (const QString& key : keys)
    {
        if (key.toLower().contains(keyword))
        {   // cppcheck-suppress useStlAlgorithm
            result = dataMap[key];
            break;
        }
    }

    if (result.isEmpty())
    {
        if (!m_exifToolMetadataCache.contains(settings.fileUrl))
        {
            QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

            if (parser->exifToolAvailable() && parser->load(settings.fileUrl.toLocalFile()))
            {
                m_exifToolMetadataCache.insert(settings.fileUrl, parser->currentData());
            }
        }

        const ExifToolParser::ExifToolData& parsed = m_exifToolMetadataCache.value(settings.fileUrl);
        ExifToolParser::ExifToolData::const_iterator it;

        for (it = parsed.constBegin() ; it != parsed.constEnd() ; ++it)
        {
            if (it.key().toLower() == keyword)
            {
                result = it.value()[0].toString();
                break;
            }
        }
    }

    result.replace(QLatin1Char('/'), QLatin1Char('_'));

    return result;
}

} // namespace Digikam

#include "moc_metadataoption.cpp"
