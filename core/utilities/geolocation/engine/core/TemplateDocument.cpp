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

#include "TemplateDocument.h"

// Qt includes

#include <QMap>
#include <QString>
#include <QFile>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#   include <QRegularExpression>
#else
#   include <QRegExp>
#endif

// Local includes

#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN TemplateDocumentPrivate
{
public:

    TemplateDocumentPrivate() = default;

    static void processTemplateIncludes(QString& input);

public:

    QString                templateText;
    QMap<QString, QString> templateEntries;
};

void TemplateDocumentPrivate::processTemplateIncludes(QString& input)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QRegularExpression rx(QString::fromUtf8("%!\\{([^}]*)\\}%"));

    QStringList includes;
    int pos = 0;

    while ((pos = input.indexOf(rx, pos)) != -1)
    {
        QRegularExpressionMatch regMatch = rx.match(input);
        includes << regMatch.captured(1);
        pos += regMatch.capturedLength();
    }

#else

    QRegExp rx(QString::fromUtf8("%!\\{([^}]*)\\}%"));

    QStringList includes;
    int pos = 0;

    while ((pos = rx.indexIn(input, pos)) != -1)
    {
        includes << rx.cap(1);
        pos += rx.matchedLength();
    }

#endif

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Template Includes" << includes;

    for (const QString& include : std::as_const(includes))
    {
        QFile includeFile(QLatin1String(":/htmlfeatures/includes/") + include + QLatin1String(".inc"));

        if (includeFile.open(QIODevice::ReadOnly))
        {
            input.replace(QLatin1String("%!{") + include + QLatin1String("}%"),
                          QString::fromUtf8(includeFile.readAll()));
        }

        else
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "[WARNING] Can't process template include" << include;
        }
    }
}

TemplateDocument::TemplateDocument()
    : d(new TemplateDocumentPrivate)
{
}

TemplateDocument::TemplateDocument(const QString& templateText)
    : d(new TemplateDocumentPrivate)
{
    setTemplate(templateText);
}

TemplateDocument::~TemplateDocument()
{
    delete d;
}

void TemplateDocument::setTemplate(const QString& newTemplateText)
{
    d->templateText = newTemplateText;
}

QString TemplateDocument::value(const QString& key) const
{
    return d->templateEntries[key];
}

void TemplateDocument::setValue(const QString& key, const QString& value)
{
    d->templateEntries[key] = value;
}

QString& TemplateDocument::operator[](const QString& key)
{
    return d->templateEntries[key];
}

QString TemplateDocument::finalText() const
{
    QString ready     = d->templateText;
    typedef QMap<QString, QString>::ConstIterator ConstIterator;
    ConstIterator end = d->templateEntries.constEnd();

    for (ConstIterator i = d->templateEntries.constBegin() ; i != end ; i++)
    {
        ready.replace(QLatin1Char('%') + i.key() + QLatin1Char('%'), i.value());
    }

    d->processTemplateIncludes(ready);

    return ready;
}

} // namespace Marble
