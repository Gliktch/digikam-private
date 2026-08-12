/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-07-16
 * Description : unit test for the worker thread naming of ActionJob
 *
 * SPDX-FileCopyrightText: 2026 by Andreas Winther <git.tumble747@simplelogin.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QThread>
#include <QTest>

// Local includes

#include "actionthreadbase.h"
#include "digikam_debug.h"

// OS includes to read back the current thread name, mirroring the
// platforms supported by ActionThreadBase::setCurrentThreadName().

#if defined(Q_OS_LINUX)
#   include <sys/prctl.h>
#elif defined(Q_OS_MACOS) || defined(Q_OS_NETBSD)
#   include <pthread.h>
#elif defined(Q_OS_WIN)
#   include <windows.h>
#   include <processthreadsapi.h>
#endif

using namespace Digikam;

/**
 * @brief Read back the name of the calling thread as recorded by the OS.
 * Counterpart of ActionThreadBase::setCurrentThreadName().
 */
static QString currentThreadNameFromOS()
{
    QString name;

#if defined(Q_OS_LINUX)

    char buf[16] = { 0 };

    if (prctl(PR_GET_NAME, (unsigned long)buf, 0, 0, 0) == 0)
    {
        name = QString::fromLatin1(buf);
    }

#elif defined(Q_OS_MACOS) || defined(Q_OS_NETBSD)

    char buf[64] = { 0 };

    if (pthread_getname_np(pthread_self(), buf, sizeof(buf)) == 0)
    {
        name = QString::fromLatin1(buf);
    }

#elif defined(Q_OS_WIN)

    PWSTR data = nullptr;

    if (SUCCEEDED(GetThreadDescription(GetCurrentThread(), &data)) && data)
    {
        name = QString::fromWCharArray(data);
        LocalFree(data);
    }

#endif

    return name;
}

// -------------------------------------------------------

/**
 * @brief Minimal ActionJob which records the thread name applied by
 * ActionJob::run().
 */
class NameRecordingJob : public ActionJob
{
    Q_OBJECT

public:

    QString appliedName;

    void run() override
    {
        ActionJob::run();       // To customize thread name

        appliedName = currentThreadNameFromOS();
    }
};

/**
 * @brief The class names below are at most 15 characters long, so they
 * survive the Linux kernel thread name limit untruncated.
 */
class FallbackTestJob : public NameRecordingJob
{
    Q_OBJECT
};

namespace DigikamTests
{

class ScopedTestJob : public NameRecordingJob
{
    Q_OBJECT
};

} // namespace DigikamTests

// -------------------------------------------------------

class ActionJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testExplicitObjectNameIsApplied();
    void testUnnamedJobFallsBackToClassName();
    void testNamespacePrefixIsStrippedFromClassName();

private:

    QString runOnWorkerThread(NameRecordingJob& job);
};

QString ActionJobTest::runOnWorkerThread(NameRecordingJob& job)
{
    QScopedPointer<QThread> thread(QThread::create([&job]()
        {
            job.run();
        }
    ));

    thread->start();

    if (!thread->wait(5000))
    {
        return QString();
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "Thread name applied by ActionJob::run():"
                               << job.appliedName;

    return job.appliedName;
}

void ActionJobTest::testExplicitObjectNameIsApplied()
{
#if !defined(Q_OS_LINUX) && !defined(Q_OS_MACOS) && !defined(Q_OS_NETBSD) && !defined(Q_OS_WIN)

    QSKIP("Reading back the current thread name is not supported on this platform");

#endif

    // At most 15 characters, so the name fits untruncated on every platform.

    const QString explicitName = QLatin1String("digiKamTestJob");

    FallbackTestJob job;
    job.setObjectName(explicitName);

    QCOMPARE(runOnWorkerThread(job), explicitName);
}

void ActionJobTest::testUnnamedJobFallsBackToClassName()
{
#if !defined(Q_OS_LINUX) && !defined(Q_OS_MACOS) && !defined(Q_OS_NETBSD) && !defined(Q_OS_WIN)

    QSKIP("Reading back the current thread name is not supported on this platform");

#endif

    FallbackTestJob job;

    QVERIFY(job.objectName().isEmpty());
    QCOMPARE(runOnWorkerThread(job), QLatin1String("FallbackTestJob"));
}

void ActionJobTest::testNamespacePrefixIsStrippedFromClassName()
{
#if !defined(Q_OS_LINUX) && !defined(Q_OS_MACOS) && !defined(Q_OS_NETBSD) && !defined(Q_OS_WIN)

    QSKIP("Reading back the current thread name is not supported on this platform");

#endif

    DigikamTests::ScopedTestJob job;

    QVERIFY(job.objectName().isEmpty());
    QCOMPARE(QString::fromLatin1(job.metaObject()->className()),
             QLatin1String("DigikamTests::ScopedTestJob"));
    QCOMPARE(runOnWorkerThread(job), QLatin1String("ScopedTestJob"));
}

// -------------------------------------------------------

QTEST_GUILESS_MAIN(ActionJobTest)

#include "actionjob_utest.moc"
