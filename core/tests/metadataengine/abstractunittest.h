/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-11-03
 * Description : An abstract digiKam unit-test template.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QDir>
#include <QTest>
#include <QString>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "wstoolutils.h"
#include "exiftoolparser.h"
#include "dtestdatadir.h"

#ifdef HAVE_IMAGE_MAGICK

// Pragma directives to reduce warnings from ImageMagick header files.
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wignored-qualifiers"
#       pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wignored-qualifiers"
#       pragma clang diagnostic ignored "-Wkeyword-macro"
#   endif

#   include <Magick++.h>
using namespace Magick;

// Restore warnings
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif // HAVE_IMAGE_MAGICK

using namespace Digikam;

class AbstractUnitTest : public QObject
{
    Q_OBJECT

public:

    explicit AbstractUnitTest(QObject* const parent = nullptr)
        : QObject              (parent),
          m_originalImageFolder(DTestDataDir::TestData(QString::fromUtf8("core/tests/metadataengine"))
                                .root().path() + QLatin1Char('/'))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << m_originalImageFolder;
    }

protected Q_SLOTS:

    /// Re-implemented from QTest framework

    virtual void initTestCase()
    {

#ifdef HAVE_IMAGE_MAGICK

        qCDebug(DIGIKAM_TESTS_LOG) << "Init ImageMagick";
        InitializeMagick(nullptr);

#endif

        MetaEngine::initializeExiv2();
        qCDebug(DIGIKAM_TESTS_LOG) << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
        m_tempPath    = QString::fromLatin1(QTest::currentAppName());
        m_tempPath.replace(QLatin1String("./"), QString());

        QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));
        m_hasExifTool = parser->exifToolAvailable();
    }

    /// Re-implemented from QTest framework

    virtual void init()
    {
        m_tempDir = WSToolUtils::makeTemporaryDir(m_tempPath.toLatin1().data());
    }

    /// Re-implemented from QTest framework

    virtual void cleanup()
    {
        WSToolUtils::removeTemporaryDir(m_tempPath.toLatin1().data());

#ifdef HAVE_IMAGE_MAGICK

#   if MagickLibVersion >= 0x693

        qCDebug(DIGIKAM_TESTS_LOG) << "Terminate ImageMagick";
        TerminateMagick();

#   endif

#endif

    }

    /// Re-implemented from QTest framework

    virtual void cleanupTestCase()
    {
    }

protected:

    QString m_tempPath;               ///< The temporary path to store file to process unit test.
    QDir    m_tempDir;                ///< Same that previous as QDir object.
    bool    m_hasExifTool = false;    ///< ExifTool is available in unit test.
    QString m_originalImageFolder;    ///< The path to original files to process by unit test, and copied to the temporary directory. Original files still in read only.
};
