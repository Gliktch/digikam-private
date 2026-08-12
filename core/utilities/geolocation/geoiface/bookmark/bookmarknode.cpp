/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : a node container for bookmarks
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bookmarknode.h"

// Qt includes

#include <QPointer>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN BookmarkNode::Private
{
public:

    Private() = default;

public:

    BookmarkNode*        parent     = nullptr;
    Type                 type       = BookmarkNode::Root;
    QList<BookmarkNode*> children;
};

BookmarkNode::BookmarkNode(BookmarkNode::Type type, BookmarkNode* const parent)
    : QObject(nullptr),
      d      (new Private)
{
    expanded  = false;
    d->parent = parent;
    d->type   = type;

    if (parent)
    {
        parent->add(this);
    }
}

BookmarkNode::~BookmarkNode()
{
    if (d->parent)
    {
        d->parent->remove(this);
    }

    while (d->children.size())
    {
        delete d->children.takeFirst();
    }

    delete d;
}

bool BookmarkNode::operator==(const BookmarkNode& other) const
{
    if (
        (url                 != other.url)           ||
        (title               != other.title)         ||
        (desc                != other.desc)          ||
        (expanded            != other.expanded)      ||
        (dateAdded           != other.dateAdded)     ||
        (d->type             != other.d->type)       ||
        (d->children.count() != other.d->children.count())
       )
    {
        return false;
    }

    for (int i = 0 ; i < d->children.count() ; ++i)
    {
        if (!((*(d->children[i])) == (*(other.d->children[i]))))
        {
            return false;
        }
    }

    return true;
}

BookmarkNode::Type BookmarkNode::type() const
{
    return d->type;
}

void BookmarkNode::setType(Type type)
{
    d->type = type;
}

QList<BookmarkNode*> BookmarkNode::children() const
{
    return d->children;
}

BookmarkNode* BookmarkNode::parent() const
{
    return d->parent;
}

void BookmarkNode::add(BookmarkNode* const child, int offset)
{
    Q_ASSERT(child->d->type != Root);

    if (child->d->parent)
    {
        child->d->parent->remove(child);
    }

    child->d->parent = this;

    if (offset == -1)
    {
        offset = d->children.size();
    }

    d->children.insert(offset, child);
}

void BookmarkNode::remove(BookmarkNode* const child)
{
    child->d->parent = nullptr;
    d->children.removeAll(child);
}

// -------------------------------------------------------

BookmarkNode* XbelReader::read(const QString& fileName)
{
    QFile file(fileName);

    if (!file.exists() || !file.open(QFile::ReadOnly))
    {
        BookmarkNode* const root   = new BookmarkNode(BookmarkNode::Root);
        BookmarkNode* const folder = new BookmarkNode(BookmarkNode::RootFolder, root);
        folder->title              = i18n("Bookmark folder");

        return root;
    }

    return read(&file, true);
}

BookmarkNode* XbelReader::read(QIODevice* const device, bool addRootFolder)
{
    BookmarkNode* const root = new BookmarkNode(BookmarkNode::Root);
    setDevice(device);

    if (readNextStartElement())
    {
        QString version = attributes().value(QLatin1String("version")).toString();

        if (
            (name() == QLatin1String("xbel")) &&
            (version.isEmpty() || (version == QLatin1String("1.0")))
           )
        {
            if (addRootFolder)
            {
                BookmarkNode* const folder = new BookmarkNode(BookmarkNode::RootFolder, root);
                folder->title              = i18n("Bookmark folder");
                readXBEL(folder);
            }
            else
            {
                readXBEL(root);
            }
        }
        else
        {
            raiseError(i18n("The file is not an XBEL version 1.0 file."));
        }
    }

    return root;
}

void XbelReader::readXBEL(BookmarkNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("xbel")));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("folder"))
        {
            readFolder(parent);
        }
        else if (name() == QLatin1String("bookmark"))
        {
            readBookmarkNode(parent);
        }
        else if (name() == QLatin1String("separator"))
        {
            readSeparator(parent);
        }
        else
        {
            skipCurrentElement();
        }
    }
}

void XbelReader::readFolder(BookmarkNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("folder")));

    QPointer<BookmarkNode> folder = new BookmarkNode(BookmarkNode::Folder, parent);
    folder->expanded              = (attributes().value(QLatin1String("folded")) == QLatin1String("no"));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("title"))
        {
            readTitle(folder);
        }
        else if (name() == QLatin1String("desc"))
        {
            readDescription(folder);
        }
        else if (name() == QLatin1String("folder"))
        {
            readFolder(folder);
        }
        else if (name() == QLatin1String("bookmark"))
        {
            readBookmarkNode(folder);
        }
        else if (name() == QLatin1String("separator"))
        {
            readSeparator(folder);
        }
        else
        {
            skipCurrentElement();
        }
    }
}

void XbelReader::readTitle(BookmarkNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("title")));

    parent->title = readElementText();
}

void XbelReader::readDescription(BookmarkNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("desc")));

    parent->desc = readElementText();
}

void XbelReader::readSeparator(BookmarkNode* const parent)
{
    new BookmarkNode(BookmarkNode::Separator, parent);

    // empty elements have a start and end element

    readNext();
}

void XbelReader::readBookmarkNode(BookmarkNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("bookmark")));

    BookmarkNode* const bookmark = new BookmarkNode(BookmarkNode::Bookmark, parent);
    bookmark->url                = attributes().value(QLatin1String("href")).toString();
    QString date                 = attributes().value(QLatin1String("added")).toString();
    bookmark->dateAdded          = QDateTime::fromString(date, Qt::ISODate);
    bookmark->desc               = attributes().value(QLatin1String("desc")).toString();

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("title"))
        {
            readTitle(bookmark);
        }
        else
        {
            skipCurrentElement();
        }
    }

    if (bookmark->title.isEmpty())
    {
        bookmark->title = i18n("Unknown title");
    }
}

// -------------------------------------------------------

XbelWriter::XbelWriter()
{
    setAutoFormatting(true);
}

bool XbelWriter::write(const QString& fileName, const BookmarkNode* const root)
{
    QFile file(fileName);

    if (!root || !file.open(QFile::WriteOnly))
    {
        return false;
    }

    return write(&file, root);
}

bool XbelWriter::write(QIODevice* const device, const BookmarkNode* const root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));

    if ((root->type() == BookmarkNode::Root) && !root->children().isEmpty())
    {
        const BookmarkNode* const rootFolder = root->children().constFirst();

        for (int i = 0  ; i < rootFolder->children().count() ; ++i)
        {
            writeItem(rootFolder->children().at(i));
        }
    }
    else
    {
        writeItem(root);
    }

    writeEndDocument();

    return true;
}

void XbelWriter::writeItem(const BookmarkNode* const parent)
{
    switch (parent->type())
    {
        case BookmarkNode::Folder:
        {
            writeStartElement(QLatin1String("folder"));
            writeAttribute(QLatin1String("folded"), parent->expanded ? QLatin1String("no") : QLatin1String("yes"));
            writeTextElement(QLatin1String("title"), parent->title);

            for (int i = 0 ; i < parent->children().count() ; ++i)
            {
                writeItem(parent->children().at(i));
            }

            writeEndElement();
            break;
        }

        case BookmarkNode::Bookmark:
        {
            writeStartElement(QLatin1String("bookmark"));

            if (!parent->url.isEmpty())
            {
                writeAttribute(QLatin1String("href"), parent->url);
            }

            if (parent->dateAdded.isValid())
            {
                writeAttribute(QLatin1String("added"), parent->dateAdded.toString(Qt::ISODate));
            }

            if (!parent->desc.isEmpty())
            {
                writeAttribute(QLatin1String("desc"), parent->desc);
            }

            writeTextElement(QLatin1String("title"), parent->title);

            writeEndElement();
            break;
        }

        case BookmarkNode::Separator:
        {
            writeEmptyElement(QLatin1String("separator"));
            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam

#include "moc_bookmarknode.cpp"
