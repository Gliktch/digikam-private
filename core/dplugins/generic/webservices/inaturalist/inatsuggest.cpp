/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inatsuggest.h"

// Qt includes

#include <QLabel>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "inattalker.h"
#include "inatutils.h"

namespace DigikamGenericINatPlugin
{

enum
{
    ITEM_PHOTO_IDX = 0,
    ITEM_NAME_IDX  = 1
};

struct Q_DECL_HIDDEN TaxonAndFlags
{
    explicit TaxonAndFlags(const Taxon& taxon,
                           bool visuallySimilar = false,
                           bool seenNearby = false)
        : m_taxon          (taxon),
          m_seenNearby     (seenNearby),
          m_visuallySimilar(visuallySimilar)
    {
    }

    Taxon m_taxon;
    bool  m_seenNearby      = false;
    bool  m_visuallySimilar = false;
};

struct Q_DECL_HIDDEN Completions
{
    explicit Completions(bool fromVision)
        : m_fromVision(fromVision)
    {
    }

    Taxon                m_commonAncestor;
    QList<TaxonAndFlags> m_taxa;
    bool                 m_fromVision       = false;
};

// ----------------------------------------------------------------------------

class Q_DECL_HIDDEN SuggestTaxonCompletion::Private
{
public:

    Private() = default;

public:

    TaxonEdit*                    editor            = nullptr;
    INatTalker*                   talker            = nullptr;
    QTreeWidget*                  popup             = nullptr;
    bool                          fromVision        = false;
    QVector<Taxon>                completionTaxa;
    QTimer                        timer;
    QHash<QUrl, QTreeWidgetItem*> url2item;
};

SuggestTaxonCompletion::SuggestTaxonCompletion(TaxonEdit* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->editor = parent;
    d->popup  = new QTreeWidget;
    d->popup->setWindowFlags(Qt::Popup);
    d->popup->setFocusPolicy(Qt::NoFocus);
    d->popup->setFocusProxy(parent);
    d->popup->setMouseTracking(true);

    d->popup->setUniformRowHeights(true);
    d->popup->setRootIsDecorated(false);
    d->popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    d->popup->setSelectionBehavior(QTreeWidget::SelectRows);
    d->popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->popup->header()->hide();

    d->popup->installEventFilter(this);

    connect(d->popup, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(slotDoneCompletion()));

    d->timer.setSingleShot(true);
    d->timer.setInterval(500);

    connect(&d->timer, SIGNAL(timeout()),
            this, SLOT(slotAutoSuggest()));

    connect(d->editor, SIGNAL(textEdited(QString)),
            SLOT(slotTextEdited(QString)));
}

SuggestTaxonCompletion::~SuggestTaxonCompletion()
{
    delete d->popup;
    delete d;
}

void SuggestTaxonCompletion::slotTextEdited(const QString&)
{
    Q_EMIT signalTaxonDeselected();

    d->timer.start();
}

void SuggestTaxonCompletion::setTalker(INatTalker* const inatTalker)
{
    d->talker = inatTalker;

    connect(d->talker, SIGNAL(signalTaxonAutoCompletions(AutoCompletions)),
            this, SLOT(slotTaxonAutoCompletions(AutoCompletions)));

    connect(d->talker, SIGNAL(signalComputerVisionResults(ImageScores)),
            this, SLOT(slotComputerVisionResults(ImageScores)));

    connect(d->editor, SIGNAL(inFocus()),
            this, SLOT(slotInFocus()));

    connect(d->talker, SIGNAL(signalLoadUrlSucceeded(QUrl,QByteArray)),
            this, SLOT(slotImageLoaded(QUrl,QByteArray)));
}

void SuggestTaxonCompletion::slotInFocus()
{
    Q_EMIT signalTaxonDeselected();

    d->timer.start();
}

bool SuggestTaxonCompletion::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj != d->popup)
    {
        return false;
    }

    if (ev->type() == QEvent::MouseButtonPress)
    {
        d->popup->hide();
        d->editor->setFocus();

        return true;
    }

    if (ev->type() == QEvent::KeyPress)
    {
        bool consumed = false;
        int key       = static_cast<QKeyEvent*>(ev)->key();

        switch (key)
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                slotDoneCompletion();
                consumed = true;
                break;
            }

            case Qt::Key_Escape:
            {
                d->editor->setFocus();
                d->popup->hide();
                consumed = true;
                break;
            }

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            {
                break;
            }

            default:
            {
                d->editor->setFocus();
                d->editor->event(ev);
                d->popup->hide();
                break;
            }
        }

        return consumed;
    }

    return false;
}

void SuggestTaxonCompletion::taxon2Item(const Taxon& taxon,
                                        QTreeWidgetItem* item,
                                        const QString& info)
{
    QString htmlText = taxon.htmlName() + QLatin1String("<br>") +
                       taxon.commonName() +
                       QLatin1String("<br><font color=\"#74ac00\">") +
                       info + QLatin1String("</font>");
    d->popup->setItemWidget(item, ITEM_NAME_IDX, new QLabel(htmlText));

    // photo

    const QUrl& photoUrl = taxon.squareUrl();

    if (!photoUrl.isEmpty())
    {
        d->url2item.insert(photoUrl, item);
        d->talker->loadUrl(photoUrl);
    }
}

void SuggestTaxonCompletion::showCompletion(const Completions& choices)
{
    d->popup->setUpdatesEnabled(false);
    d->popup->clear();
    d->popup->setIconSize(QSize(75, 75));
    d->fromVision = choices.m_fromVision;
    int columns   = choices.m_taxa.isEmpty() ? 1 : 2;
    d->popup->setColumnCount(columns);
    d->url2item.clear();

    if (choices.m_commonAncestor.isValid())
    {
        const Taxon& taxon = choices.m_commonAncestor;

        Q_ASSERT(choices.m_fromVision);

        auto item          = new QTreeWidgetItem(d->popup);
        taxon2Item(taxon, item, i18n("We are pretty sure it is in this %1.",
                                     localizedTaxonomicRank(taxon.rank())));
    }

    for (const auto& choice : choices.m_taxa)
    {
        QString info;

        if      (choice.m_visuallySimilar && choice.m_seenNearby)
        {
            info = i18n("Visually Similar") + QLatin1String(" / ") +
                   i18n("Seen Nearby");
        }
        else if (choice.m_visuallySimilar)
        {
            info = i18n("Visually Similar");
        }
        else if (choice.m_seenNearby)
        {
            info = i18n("Seen Nearby");
        }

        auto item = new QTreeWidgetItem(d->popup);
        taxon2Item(choice.m_taxon, item, info);
    }

    if (choices.m_taxa.isEmpty())
    {
        auto item  = new QTreeWidgetItem(d->popup);
        QFont font = item->font(0);
        font.setBold(true);
        item->setForeground(0, QColor(Qt::red));
        item->setText(0, i18n("invalid name"));
        item->setFont(0, font);
    }

    d->popup->setCurrentItem(d->popup->topLevelItem(0));

    for (int i = 0 ; i < columns ; ++i)
    {
        d->popup->resizeColumnToContents(i);
    }

    d->popup->setUpdatesEnabled(true);
    d->popup->setMinimumWidth(d->editor->width());
    d->popup->move(d->editor->mapToGlobal(QPoint(0, d->editor->height())));
    d->popup->setFocus();
    d->popup->show();
}

void SuggestTaxonCompletion::slotDoneCompletion()
{
    d->timer.stop();
    d->url2item.clear();
    d->popup->hide();
    d->editor->setFocus();

    if (d->completionTaxa.count() == 0)
    {
        return;
    }

    QTreeWidgetItem* const item = d->popup->currentItem();

    if (item)
    {
        int idx = item->treeWidget()->indexOfTopLevelItem(item);

        if (idx < d->completionTaxa.count())
        {
            const Taxon& taxon = d->completionTaxa[idx];

            if (taxon.commonName().isEmpty())
            {
                d->editor->setText(taxon.name());
            }
            else
            {
                // combine scientific name and common name

                d->editor->setText(taxon.name() + QLatin1String(" (") +
                                   taxon.commonName() + QLatin1String(")"));
            }

            QMetaObject::invokeMethod(d->editor, "returnPressed");

            Q_EMIT signalTaxonSelected(taxon, d->fromVision);
        }
    }
}

QString SuggestTaxonCompletion::getText() const
{
    QString str = d->editor->text().simplified();

    // When we have "scientific name (common name)" we only
    // send the scientific name to auto-completion.

    int idx = str.indexOf(QLatin1String(" ("));

    if (idx >= 0)
    {
        str.truncate(idx);
    }

    return str;
}

void SuggestTaxonCompletion::slotAutoSuggest()
{
    QString str = getText();

    if (str.size() > 0)
    {
        d->talker->taxonAutoCompletions(str);
    }
    else
    {
        Q_EMIT signalComputerVision();
    }
}

void SuggestTaxonCompletion::slotPreventSuggest()
{
    d->timer.stop();
}

void SuggestTaxonCompletion::slotTaxonAutoCompletions(const AutoCompletions& taxa)
{
    if (getText() != taxa.first)
    {
        return;
    }

    Completions completions(false);

    d->completionTaxa.clear();

    for (const auto& taxon : taxa.second)
    {
        completions.m_taxa << TaxonAndFlags(taxon);
        d->completionTaxa.append(taxon);
    }

    showCompletion(completions);
}

void SuggestTaxonCompletion::slotComputerVisionResults(const ImageScores& scores)
{
    if (!d->editor->text().simplified().isEmpty())
    {
        return;
    }

    Completions completions(true);

    d->completionTaxa.clear();

    for (const auto& score : scores.second)
    {
        if (score.getTaxon().ancestors().isEmpty())
        {
            Q_ASSERT(!completions.m_commonAncestor.isValid());

            completions.m_commonAncestor = score.getTaxon();
        }
        else
        {
            completions.m_taxa << TaxonAndFlags(score.getTaxon(),
                                                score.visuallySimilar(),
                                                score.seenNearby());
        }

        d->completionTaxa.append(score.getTaxon());
    }

    showCompletion(completions);
}

void SuggestTaxonCompletion::slotImageLoaded(const QUrl& url, const QByteArray& data)
{
    if (d->url2item.contains(url))
    {
        QTreeWidgetItem* const item = d->url2item[url];
        QImage image;
        image.loadFromData(data);
        QIcon icon(QPixmap::fromImage(image));
        item->setIcon(ITEM_PHOTO_IDX, icon);
        d->popup->resizeColumnToContents(ITEM_PHOTO_IDX);
        d->popup->resizeColumnToContents(ITEM_NAME_IDX);
    }
}

} // namespace DigikamGenericINatPlugin

#include "moc_inatsuggest.cpp"
