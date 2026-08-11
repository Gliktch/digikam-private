/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

// C++ includes

#include <cctype>
#include <cstdio>

class PrivacyIndependentRecoveryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPasswordRecoveryRestoresExactOriginals();
    void testMasterKeyRecoveryRestoresExactOriginals();
    void testWrongPasswordIsRejected();
};

namespace
{

const QByteArray PrimaryBytes("synthetic original payload\0with NUL bytes", 39);
const QByteArray SidecarBytes("synthetic sidecar payload", 25);
const QByteArray Password("external-recovery-passphrase");

QString toolPath(const QString& name)
{
    const QString found = QStandardPaths::findExecutable(name);

    return found.isEmpty()
         ? QLatin1String("/usr/bin/") + name
         : found;
}

bool fileExists(const QString& path)
{
    return QFileInfo::exists(path);
}

bool writeFile(const QString& path, const QByteArray& contents)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(contents) == contents.size()) && file.flush();
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

QByteArray sha256(const QByteArray& bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

int runProcess(const QString& program, const QStringList& arguments,
               const QByteArray& standardInput, QByteArray* standardOutput,
               QByteArray* standardError, int timeoutMs = 60000)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();

    if (!process.waitForStarted(10000))
    {
        return -1000;
    }

    if (!standardInput.isEmpty())
    {
        process.write(standardInput);
        process.closeWriteChannel();
    }

    if (!process.waitForFinished(timeoutMs))
    {
        process.kill();
        process.waitForFinished(5000);
        return -1001;
    }

    if (standardOutput)
    {
        *standardOutput = process.readAllStandardOutput();
    }

    if (standardError)
    {
        *standardError = process.readAllStandardError();
    }

    return process.exitCode();
}

bool unmount(const QString& mountPoint)
{
    const QStringList tools = {
        QLatin1String("fusermount3"),
        QLatin1String("fusermount")
    };

    for (const QString& tool : tools)
    {
        const QString path = toolPath(tool);

        if (!fileExists(path))
        {
            continue;
        }

        for (const QString& flag : { QStringLiteral("-u"),
                                     QStringLiteral("-uz") })
        {
            if (runProcess(path, { flag, mountPoint }, {}, {}, {}) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool mountStore(const QString& cipherDirectory, const QString& mountPoint,
                const QByteArray& secret, QString* detail)
{
    const QString gocryptfs = toolPath(QLatin1String("gocryptfs"));

    if (!fileExists(gocryptfs))
    {
        *detail = QStringLiteral("gocryptfs is unavailable");
        return false;
    }

    QByteArray standardError;
    const int exitCode = runProcess(
        gocryptfs, { QLatin1String("-q"), cipherDirectory, mountPoint },
        secret + '\n', nullptr, &standardError);

    if (detail)
    {
        *detail = QString::fromUtf8(standardError);
    }

    return (exitCode == 0);
}

bool prepareStrongStore(const QString& gocryptfs, const QString& cipherDirectory,
                        const QString& mountPoint,
                        const QByteArray& password, QString* detail)
{
    if (!QDir().mkpath(cipherDirectory))
    {
        *detail = QStringLiteral("could not create the cipher directory");
        return false;
    }

    QByteArray standardError;
    const int initCode = runProcess(
        gocryptfs,
        { QLatin1String("-init"), QLatin1String("-q"),
          QLatin1String("-scryptn=10"), cipherDirectory },
        password + '\n', nullptr, &standardError);

    if ((initCode != 0) || !fileExists(cipherDirectory + QLatin1String("/gocryptfs.conf")))
    {
        *detail = QString::fromUtf8(standardError);
        return false;
    }

    if (!mountStore(cipherDirectory, mountPoint, password, detail))
    {
        return false;
    }

    if (!writeFile(QDir(mountPoint).filePath(
                       QLatin1String("originals/50000000-0000-0000-0000-000000000001/0-photo.jpg")),
                   PrimaryBytes) ||
        !writeFile(QDir(mountPoint).filePath(
                       QLatin1String("originals/50000000-0000-0000-0000-000000000001/1-photo.xmp")),
                   SidecarBytes))
    {
        unmount(mountPoint);
        *detail = QStringLiteral("could not write synthetic originals");
        return false;
    }

    return unmount(mountPoint);
}

void verifyExactRestore(const QString& mountPoint, const QString& restoreDirectory)
{
    const QString containerRelative =
        QLatin1String("originals/50000000-0000-0000-0000-000000000001");

    for (const auto& expected : {
             QPair<QString, QByteArray>(
                 QLatin1String("0-photo.jpg"), PrimaryBytes),
             QPair<QString, QByteArray>(
                 QLatin1String("1-photo.xmp"), SidecarBytes)
         })
    {
        const QString plaintextPath = QDir(mountPoint).filePath(
            containerRelative + QLatin1Char('/') + expected.first);
        const QByteArray restored = readFile(plaintextPath);
        QCOMPARE(restored, expected.second);
        QCOMPARE(sha256(restored), sha256(expected.second));

        const QString restoredPath = QDir(restoreDirectory).filePath(
            expected.first);
        QVERIFY2(writeFile(restoredPath, restored),
                 qPrintable(restoredPath));
        QCOMPARE(readFile(restoredPath), expected.second);
    }
}

} // namespace

void PrivacyIndependentRecoveryTest::testPasswordRecoveryRestoresExactOriginals()
{
    const QString gocryptfs = toolPath(QLatin1String("gocryptfs"));

    if (!fileExists(gocryptfs))
    {
        QSKIP("gocryptfs is unavailable in this test environment");
    }

    if (!fileExists(QLatin1String("/dev/fuse")))
    {
        QSKIP("FUSE is unavailable in this test environment");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString cipherDirectory = directory.filePath(QLatin1String("cipher"));
    const QString mountPoint = directory.filePath(QLatin1String("mount"));
    const QString recoveredCipher = directory.filePath(QLatin1String("recovered-cipher"));
    const QString recoveredMount = directory.filePath(QLatin1String("recovered-mount"));
    const QString restoreDirectory = directory.filePath(QLatin1String("restored"));
    QVERIFY(QDir().mkpath(mountPoint));
    QVERIFY(QDir().mkpath(recoveredMount));

    QString detail;
    QVERIFY2(prepareStrongStore(gocryptfs, cipherDirectory, mountPoint,
                                Password, &detail),
             qPrintable(detail));

    // The plaintext layout must not be visible in the ciphertext tree.
    QVERIFY(!fileExists(cipherDirectory + QLatin1String("/originals")));
    QVERIFY(fileExists(cipherDirectory + QLatin1String("/gocryptfs.conf")));

    QVERIFY(QDir().mkpath(recoveredCipher));

    for (const QFileInfo& entry :
         QDir(cipherDirectory).entryInfoList(QDir::AllEntries |
                                             QDir::NoDotAndDotDot))
    {
        const QString target = QDir(recoveredCipher).filePath(entry.fileName());

        if (entry.isDir())
        {
            QVERIFY(QDir().rename(entry.absoluteFilePath(), target));
        }
        else
        {
            QVERIFY(QFile::copy(entry.absoluteFilePath(), target));
        }
    }

    // A wrong password must not open the copied store.
    QVERIFY(!mountStore(recoveredCipher, recoveredMount,
                        QByteArray("wrong-password"), &detail));

    QVERIFY2(mountStore(recoveredCipher, recoveredMount, Password, &detail),
             qPrintable(detail));
    verifyExactRestore(recoveredMount, restoreDirectory);
    QVERIFY(unmount(recoveredMount));
    QVERIFY(QDir(mountPoint).isEmpty());
}

void PrivacyIndependentRecoveryTest::testMasterKeyRecoveryRestoresExactOriginals()
{
    const QString gocryptfs = toolPath(QLatin1String("gocryptfs"));
    const QString xray = toolPath(QLatin1String("gocryptfs-xray"));

    if (!fileExists(gocryptfs) || !fileExists(xray))
    {
        QSKIP("gocryptfs tooling is unavailable in this test environment");
    }

    if (!fileExists(QLatin1String("/dev/fuse")))
    {
        QSKIP("FUSE is unavailable in this test environment");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString cipherDirectory = directory.filePath(QLatin1String("cipher"));
    const QString mountPoint = directory.filePath(QLatin1String("mount"));
    const QString recoveredCipher = directory.filePath(QLatin1String("recovered-cipher"));
    const QString recoveredMount = directory.filePath(QLatin1String("recovered-mount"));
    const QString restoreDirectory = directory.filePath(QLatin1String("restored"));
    QVERIFY(QDir().mkpath(mountPoint));
    QVERIFY(QDir().mkpath(recoveredMount));

    QString detail;
    QVERIFY2(prepareStrongStore(gocryptfs, cipherDirectory, mountPoint,
                                Password, &detail),
             qPrintable(detail));

    QByteArray xrayOutput;
    QByteArray xrayError;
    const int xrayCode = runProcess(
        xray,
        { QLatin1String("-dumpmasterkey"),
          cipherDirectory + QLatin1String("/gocryptfs.conf") },
        Password + '\n', &xrayOutput, &xrayError);
    QVERIFY2(xrayCode == 0, qPrintable(QString::fromUtf8(xrayError)));

    QByteArray masterKey;

    for (const QByteArray& line : xrayOutput.split('\n'))
    {
        QByteArray candidate = line.trimmed();
        const int separator = candidate.lastIndexOf(':');

        if (separator >= 0)
        {
            candidate = candidate.mid(separator + 1).trimmed();
        }

        if (candidate.size() == 64)
        {
            bool hexadecimal = true;

            for (const char value : candidate)
            {
                if (!std::isxdigit(static_cast<unsigned char>(value)))
                {
                    hexadecimal = false;
                    break;
                }
            }

            if (hexadecimal)
            {
                masterKey = candidate;
                break;
            }
        }
    }

    QVERIFY2(!masterKey.isEmpty(), "could not parse the exported master key");

    QVERIFY(QDir().mkpath(recoveredCipher));

    for (const QFileInfo& entry :
         QDir(cipherDirectory).entryInfoList(QDir::AllEntries |
                                             QDir::NoDotAndDotDot))
    {
        const QString target = QDir(recoveredCipher).filePath(entry.fileName());

        if (entry.isDir())
        {
            QVERIFY(QDir().rename(entry.absoluteFilePath(), target));
        }
        else
        {
            QVERIFY(QFile::copy(entry.absoluteFilePath(), target));
        }
    }

    QByteArray mountError;
    const int mountCode = runProcess(
        gocryptfs,
        { QLatin1String("-q"), QLatin1String("-masterkey=stdin"),
          recoveredCipher, recoveredMount },
        masterKey + '\n', nullptr, &mountError);
    QVERIFY2(mountCode == 0,
             qPrintable(QString::fromUtf8(mountError)));
    verifyExactRestore(recoveredMount, restoreDirectory);
    QVERIFY(unmount(recoveredMount));
}

void PrivacyIndependentRecoveryTest::testWrongPasswordIsRejected()
{
    const QString gocryptfs = toolPath(QLatin1String("gocryptfs"));

    if (!fileExists(gocryptfs))
    {
        QSKIP("gocryptfs is unavailable in this test environment");
    }

    if (!fileExists(QLatin1String("/dev/fuse")))
    {
        QSKIP("FUSE is unavailable in this test environment");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString cipherDirectory = directory.filePath(QLatin1String("cipher"));
    const QString mountPoint = directory.filePath(QLatin1String("mount"));
    QVERIFY(QDir().mkpath(mountPoint));

    QString detail;
    QVERIFY2(prepareStrongStore(gocryptfs, cipherDirectory, mountPoint,
                                Password, &detail),
             qPrintable(detail));

    QByteArray error;
    const int exitCode = runProcess(
        gocryptfs,
        { QLatin1String("-q"), cipherDirectory, mountPoint },
        QByteArray("wrong-password") + '\n', nullptr, &error);
    QVERIFY(exitCode != 0);
    QVERIFY(QDir(mountPoint).isEmpty());
}

QTEST_GUILESS_MAIN(PrivacyIndependentRecoveryTest)

#include "privacyindependentrecovery_utest.moc"
