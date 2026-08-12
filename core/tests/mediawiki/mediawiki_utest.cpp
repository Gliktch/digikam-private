/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QObject>
#include <QString>
#include <QUrl>
#include <QTest>

// Local includes

#include "mediawiki_iface.h"

using MediaWiki::Iface;

class Q_DECL_HIDDEN IfaceTest : public QObject
{
    Q_OBJECT

public:

    explicit IfaceTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
    }

private Q_SLOTS:

    void testConstructor()
    {
        QFETCH(QUrl, url);
        QFETCH(QString, customUserAgent);
        QFETCH(QString, userAgent);

        Iface MediaWiki(url, customUserAgent);

        QCOMPARE(MediaWiki.url(), url);
        QCOMPARE(MediaWiki.userAgent(), userAgent);
    }

    void testConstructor_data()
    {
        QTest::addColumn<QUrl>("url");
        QTest::addColumn<QString>("customUserAgent");
        QTest::addColumn<QString>("userAgent");

        QTest::newRow("") << QUrl(QStringLiteral("http://127.0.0.1:12566"))                         // krazy:exclude=insecurenet
                          << QString() << QStringLiteral("MediaWiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("commons.wikimedia.org/w/api.php"))
                          << QString() << QStringLiteral("MediaWiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("http://commons.wikimedia.org/w/api.php"))         // krazy:exclude=insecurenet
                          << QStringLiteral("test1") << QStringLiteral("test1-MediaWiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("http://commons.wikimedia.org/w/api.php/"))        // krazy:exclude=insecurenet
                          << QStringLiteral("test2") << QStringLiteral("test2-MediaWiki-silk");
    }
};

QTEST_MAIN(IfaceTest)

#include "mediawiki_utest.moc"
