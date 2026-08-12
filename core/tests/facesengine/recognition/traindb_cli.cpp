/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-06-16
 * Description : CLI test program for training database
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QThreadPool>
#include <QRunnable>

// Local includes

#include "digikam_debug.h"
#include "facialrecognition_wrapper.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"
#include "actionthreadbase.h"

using namespace Digikam;

// --------------------------------------------------------------------------------------------------

const int firstMultiplier  = 20;
const int secondMultiplier = 20;

class Q_DECL_HIDDEN Runnable : public QRunnable
{
public:

    Runnable(int number, const FacialRecognitionWrapper& db)
        : number(number),
          db(db)
    {
    }

    void run() override
    {
        ActionThreadBase::setCurrentThreadName(QLatin1String("TrainDbCliThread"));       // To customize thread name

        QImage* const image = new QImage(256, 256, QImage::Format_ARGB32);
        image->fill(Qt::red);

        Identity identity;

        // Populate database.

        for (int i = number * secondMultiplier ; i < (number * secondMultiplier + secondMultiplier) ; ++i)
        {
            QString name                            = QString::fromLatin1("face%1").arg(i);
            qCDebug(DIGIKAM_TESTS_LOG) << "Record Identity " << name << " to DB";
            QMultiMap<QString, QString> attributes;
            attributes.insert(QString::fromLatin1("name"), name);
            identity                                = db.addIdentity(attributes);
            db.train(identity, QPair<QImage*, QString>(image, QLatin1String("f00d")));
        }

        qCDebug(DIGIKAM_TESTS_LOG) << "Trained group" << number;

        // Check records in database.

        for (int i = (number * secondMultiplier) ; i < (number * secondMultiplier + secondMultiplier) ; ++i)
        {
            QString name = QString::fromLatin1("face%1").arg(i);
            identity     = db.findIdentity(QString::fromLatin1("name"), name);

            if (!identity.isNull())
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Identity " << name << " is present in DB";
            }

            else
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Identity " << name << " is absent in DB";
            }
        }
    }

public:

    const int                number;
    FacialRecognitionWrapper db;

private:

    Q_DISABLE_COPY(Runnable)
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig();
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    FacialRecognitionWrapper db;

    QThreadPool pool;
    pool.setMaxThreadCount(101);

    for (int i = 0 ; i < firstMultiplier ; ++i)
    {
        Runnable* const r = new Runnable(i, db);
        pool.start(r);
    }

    pool.waitForDone();

    // Process recognition in database.

    const QImage* const image = new QImage(256, 256, QImage::Format_ARGB32);
    QList<Identity> list      = db.recognizeFaces(QList<QImage*>() << image);

    if (!list.isEmpty())
    {
        for (const Identity& id : std::as_const(list))
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Identity" << id.attribute(QString::fromLatin1("name")) << "recognized";
        }
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No Identity recognized from DB";
    }

    return 0;
}
