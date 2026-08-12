/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of DItemsList api
 *
 * SPDX-FileCopyrightText: 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ditemslist_gui.h"

// Qt includes

#include <QGridLayout>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QImage>
#include <QTransform>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"

using namespace Digikam;

class Q_DECL_HIDDEN Task : public ActionJob
{
    Q_OBJECT

public:

    explicit Task(QObject* const parent = nullptr)
        : ActionJob(parent)
    {
    }

public:

    QString errString;
    QUrl    fileUrl;

protected:

    void run() override
    {
        Q_EMIT signalStarted();

        QImage src;
        QImage dst;

        if (m_cancel)
        {
            return;
        }

        Q_EMIT signalProgress(20);

        if (!src.load(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Loading image failed. Aborted...");
            return;
        }

        if (m_cancel)
        {
            return;
        }

        Q_EMIT signalProgress(40);

        QTransform transform;
        transform.rotate(90);

        if (m_cancel)
        {
            return;
        }

        Q_EMIT signalProgress(60);

        dst = src.transformed(transform);

        if (m_cancel)
        {
            return;
        }

        Q_EMIT signalProgress(80);

        if (!dst.save(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Saving image failed. Aborted...");
            return;
        }

        Q_EMIT signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("ActionThread"));
}

ActionThread::~ActionThread()
{
}

void ActionThread::rotate(const QList<QUrl>& list)
{
    ActionJobCollection collection;

    for (const QUrl& url : std::as_const(list))
    {
        Task* const job = new Task();
        job->fileUrl    = url;

        connect(job, &Task::signalStarted,
                this, &ActionThread::slotJobStarted);

        connect(job, &Task::signalProgress,
                this, &ActionThread::slotJobProgress);

        connect(job, &Task::signalDone,
                this, &ActionThread::slotJobDone);

        collection.insert(job, 0);

        qCDebug(DIGIKAM_TESTS_LOG) << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void ActionThread::slotJobDone()
{
    Task* const task = dynamic_cast<Task*>(sender());

    if (!task)
    {
            return;
    }

    if (task->errString.isEmpty())
    {
        Q_EMIT signalFinished(task->fileUrl);
    }
    else
    {
        Q_EMIT signalFailed(task->fileUrl, task->errString);
    }
}

void ActionThread::slotJobProgress(int p)
{
    Task* const task = dynamic_cast<Task*>(sender());

    if (!task)
    {
        return;
    }

    Q_EMIT signalProgress(task->fileUrl, p);
}

void ActionThread::slotJobStarted()
{
    Task* const task = dynamic_cast<Task*>(sender());

    if (!task)
    {
        return;
    }

    Q_EMIT signalStarting(task->fileUrl);
}

// ----------------------------------------------------------

class Q_DECL_HIDDEN DItemsListTest::Private
{
public:

    Private() = default;

public:

    QWidget*          page          = nullptr;

    QDialogButtonBox* buttons       = nullptr;
    QProgressBar*     progressBar   = nullptr;
    QPushButton*      applyBtn      = nullptr;

    DItemsList*       listView      = nullptr;

    ActionThread*     thread        = nullptr;
};

DItemsListTest::DItemsListTest(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowTitle(QString::fromUtf8("Rotate Images to 180 Degrees"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Apply |
                                                    QDialogButtonBox::Close, this);

    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const mainLayout = new QGridLayout(d->page);

    d->listView                   = new DItemsList(d->page);
    d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsRight);

    d->progressBar                = new QProgressBar(d->page);
    d->progressBar->setMaximumHeight(fontMetrics().height()+2);

    mainLayout->addWidget(d->listView,    0, 0, 1, 1);
    mainLayout->addWidget(d->progressBar, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);

    d->thread = new ActionThread(this);

    d->applyBtn = d->buttons->button(QDialogButtonBox::Apply);
    d->applyBtn->setText(QLatin1String("Rotate Items"));

    connect(d->applyBtn, &QPushButton::clicked,
            this, &DItemsListTest::slotStart);

    connect(d->buttons->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &DItemsListTest::close);

    connect(d->thread, &ActionThread::signalStarting,
            this, &DItemsListTest::slotStarting);

    connect(d->thread, &ActionThread::signalFinished,
            this, &DItemsListTest::slotFinished);

    connect(d->thread, &ActionThread::signalFailed,
            this, &DItemsListTest::slotFailed);
}

DItemsListTest::~DItemsListTest()
{
    delete d;
}

void DItemsListTest::slotStart()
{
    QList<QUrl> selectedImages = d->listView->imageUrls();

    if (selectedImages.isEmpty())
    {
        return;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << selectedImages;
    d->progressBar->setMaximum(selectedImages.count());
    d->progressBar->setValue(0);
    d->applyBtn->setEnabled(false);

    // Rotate the selected images by 180 degrees
    // It can be converted to gray scale also, just change the function here
    d->thread->rotate(selectedImages);
    d->thread->start();
}

void DItemsListTest::slotStarting(const QUrl& url)
{
    d->listView->processing(url);
}

void DItemsListTest::slotFinished(const QUrl& url)
{
    d->listView->processed(url, true);
    d->progressBar->setValue(d->progressBar->value()+1);
    d->listView->updateThumbnail(url);
}

void DItemsListTest::slotFailed(const QUrl& url, const QString&)
{
    d->listView->processed(url, false);
    d->progressBar->setValue(d->progressBar->value()+1);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DItemsListTest* const view = new DItemsListTest;
    view->show();
    app.exec();
    delete view;

    return 0;
}

#include "ditemslist_gui.moc"

#include "moc_ditemslist_gui.cpp"
