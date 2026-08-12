/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-09
 * Description : Core database searches XML queries manager
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

namespace SearchXml
{

enum Operator
{
    And,
    Or,
    AndNot,
    OrNot
};

enum Element
{
    Search,
    Group,
    GroupEnd,
    Field,
    FieldEnd,
    End
};

enum Relation
{
    Equal,
    Unequal,
    Like,
    NotLike,
    LessThan,
    GreaterThan,
    LessThanOrEqual,
    GreaterThanOrEqual,
    Interval,          // [a,b]
    IntervalOpen,      // (a,b)
    OneOf,
    AllOf,
    InTree,
    NotInTree,
    Near,
    Inside
};

template <typename T>
bool testRelation(T v1, T v2, Relation rel)
{
    if      (rel == Equal)
    {
        return (v1 == v2);
    }
    else if (rel == Unequal)
    {
        return (v1 != v2);
    }
    else if (rel == LessThan)
    {
        return (v1 < v2);
    }
    else if (rel == LessThanOrEqual)
    {
        return (v1 <= v2);
    }
    else if (rel == GreaterThan)
    {
        return (v1 > v2);
    }
    else if (rel == GreaterThanOrEqual)
    {
        return (v1 >= v2);
    }

    return false;
}

/**
 * General default values for groupOperator() and defaultFieldOperator()
 */
inline SearchXml::Operator standardGroupOperator()
{
    return SearchXml::Or;
}

inline SearchXml::Operator standardFieldOperator()
{
    return SearchXml::And;
}

inline SearchXml::Relation standardFieldRelation()
{
    return SearchXml::Equal;
}

} // namespace SearchXml

// ---------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchXmlReader : public QXmlStreamReader
{
public:

    explicit SearchXmlReader(const QString& xml);
    virtual ~SearchXmlReader() = default;

    /** Continue parsing the document. Returns the type of the current element.
     */
    virtual SearchXml::Element  readNext();

    /** Returns if the current element is a group element (start or end element).
     */
    bool                isGroupElement()        const;

    /** Returns if the current element is a field element (start or end element).
     */
    bool                isFieldElement()        const;

    /** Returns the group operator. Only valid if the current element is a group.
     */
    virtual SearchXml::Operator groupOperator() const;

    /** Returns the (optional) group caption. Only valid if the current element is a group.
     */
    virtual QString             groupCaption()  const;

    /** Returns the default field operator. This operator can be overridden by a specific fieldOperator().
     */
    SearchXml::Operator defaultFieldOperator()  const;

    /** Returns the field attributes. Only valid if the current element is a field.
     *  fieldOperator returns the default operator if the field has not specified any.
     */
    virtual SearchXml::Operator fieldOperator() const;
    virtual QString             fieldName()     const;
    virtual SearchXml::Relation fieldRelation() const;

    /** Returns the field values. Only valid if the current element is a field.
     *  This reads to the end element of the field, and converts the found
     *  text/elements to the desired output.
     */
    virtual QString             value();
    virtual int                 valueToInt();
    virtual qlonglong           valueToLongLong();
    virtual double              valueToDouble();
    virtual QDateTime           valueToDateTime();
    virtual QList<int>          valueToIntList();
    virtual QList<qlonglong>    valueToLongLongList();
    virtual QList<double>       valueToDoubleList();
    virtual QStringList         valueToStringList();
    virtual QList<QDateTime>    valueToDateTimeList();

    virtual QList<int>          valueToIntOrIntList();
    virtual QList<double>       valueToDoubleOrDoubleList();
    virtual QList<QString>      valueToStringOrStringList();

    /** General helper method: Reads XML a start element with the given
     *  name is found. The method goes to the next start element, and from
     *  there down the hierarchy, but not further up in the hierarchy.
     *  Returns false if the element is not found.
     */
    bool readToStartOfElement(const QString& name);

    /** General helper method: Reads XML until the end element of the current
        start element in reached.
     */
    void readToEndOfElement();

    /** General helper method: Reads XML until the first field
     *  of the next or first found group is reached.
     */
    void readToFirstField();

protected:

    SearchXml::Operator readOperator(const QString&, SearchXml::Operator) const;
    SearchXml::Relation readRelation(const QString&, SearchXml::Relation) const;

protected:

    SearchXml::Operator m_defaultFieldOperator;
};

// ---------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchXmlWriter : public QXmlStreamWriter
{
public:

    /**
     *  Note that SearchXmlWriter and SearchXmlGroupWriter rely on you
     *  calling the methods following the restrictions set by the documentation;
     *  Otherwise you will not produce the desired output.
     */
    explicit SearchXmlWriter();
    ~SearchXmlWriter() = default;

    /** Adds a group. Use the returned group writer to add fields.
     */
    void writeGroup();

    /** Sets the operator applied to the group as a whole "OR (field1 ... fieldn)".
     *  Default value is OR.
     */
    void setGroupOperator(SearchXml::Operator op);

    /** Sets an optional caption.
     */
    void setGroupCaption(const QString& caption);

    /** Sets the default operator for fields in this group "(field1 AND field2 AND ... fieldn)".
     *  The default operator can in each field be overridden. Default value is AND.
     */
    void setDefaultFieldOperator(SearchXml::Operator op);

    /** Adds a new field with the given name (entity) and relation, "Rating less than ...".
     *  Ensure that you closed the previous field with finishField().
     *  For a reference of valid field names, look into ItemQueryBuilder.
     *  The general rule is that names are like the database fields, but all lower-case.
     */
    void writeField(const QString& name, SearchXml::Relation relation);

    /** Adds an optional operator overriding the default field operator of the group.
     */
    void setFieldOperator(SearchXml::Operator op);

    /** Adds the value, "4" in the case of "Rating less than 4".
     */
    void writeValue(const QString& value);
    void writeValue(int value);
    void writeValue(qlonglong value);
    void writeValue(float value, int precision = 6);
    void writeValue(double value, int precision = 8);
    void writeValue(const QDateTime& dateTime);
    void writeValue(const QList<int>& valueList);
    void writeValue(const QList<qlonglong>& valueList);
    void writeValue(const QList<float>& valueList, int precision = 6);
    void writeValue(const QList<double>& valueList, int precision = 8);
    void writeValue(const QStringList& valueList);
    void writeValue(const QList<QDateTime>& valueList);

    /** Finish writing the current field.
     *  You shall call this method before adding another field, or closing the group.
     */
    void finishField();

    /** Finish the current group.
     *  You cannot add anymore fields after calling this.
     *  Note that you will want to call this before
     *  writing another group if you want the group on the same level.
     *  You can as well add nested groups and call this to close the group afterwards.
     */
    void finishGroup();

    /**
     *  Finish the XML. No further group can be added after calling this.
     *  You need to call this before you can get the resulting XML from xml().
     */
    void finish();

    /** Get the created XML. The value is only valid if finish() has been called.
     */
    QString xml() const;

    /** Returns ready-made XML for a query of type "keyword" with the specified
     *  text as keyword.
     */
    static QString keywordSearch(const QString& keyword);

protected:

    void writeOperator(const QString&, SearchXml::Operator);
    void writeRelation(const QString&, SearchXml::Relation);

protected:

    QString m_xml;
};

// ---------------------------------------------------------------------------------

namespace KeywordSearch
{

/** Splits a given string to a list of keywords.
 *  Splits at whitespace, but recognizes quotation marks
 *  to group words in a single keyword.
 */
DIGIKAM_DATABASE_EXPORT QStringList split(const QString& string);

/** Reverse of split().
 *  From a list of keywords, gives a single string for a text entry field.
 */
DIGIKAM_DATABASE_EXPORT QString merge(const QStringList& keywordList);

/** Assuming previousContent is a string
 *  as accepted by split and returned by merge,
 *  adds newEntry as another (single) keyword to the string,
 *  returning the combined result.
 */
DIGIKAM_DATABASE_EXPORT QString merge(const QString& previousContent, const QString& newEntry);

} // namespace KeywordSearch

// ---------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT KeywordSearchReader : public SearchXmlReader
{
public:

    explicit KeywordSearchReader(const QString& xml);

    /// Returns the keywords from this search, merged in a list.
    QStringList keywords();

    /// Checks if the XML is a simple keyword search, compatible with keywords().
    bool isSimpleKeywordSearch();

private:

    void    readGroup(QStringList& list);
    bool    isSimpleKeywordSearchGroup();
    QString readField();
};

// ---------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT KeywordSearchWriter : public SearchXmlWriter
{
public:

    explicit KeywordSearchWriter();

    QString xml(const QStringList& keywordList);
};

// ---------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchXmlCachingReader : public SearchXmlReader
{
public:

    /** This class has the same semantics as SearchXmlReader,
     *  but performs some caching and is thus much more relaxed than SearchXmlReader
     *  about the calling order of methods:
     *  With this class, you can access properties of a group until the next group
     *  is read, access properties and the value of a field until the next field is read,
     *  with all calls possible multiple times.
     */
    explicit SearchXmlCachingReader(const QString& xml);
    ~SearchXmlCachingReader()                           override = default;

    SearchXml::Element  readNext()                      override;

    SearchXml::Operator groupOperator()           const override;
    QString             groupCaption()            const override;

    SearchXml::Operator fieldOperator()           const override;
    QString             fieldName()               const override;
    SearchXml::Relation fieldRelation()           const override;
    QString             value()                         override;
    int                 valueToInt()                    override;
    qlonglong           valueToLongLong()               override;
    double              valueToDouble()                 override;
    QDateTime           valueToDateTime()               override;
    QList<int>          valueToIntList()                override;
    QList<qlonglong>    valueToLongLongList()           override;
    QList<double>       valueToDoubleList()             override;
    QStringList         valueToStringList()             override;
    QList<QDateTime>    valueToDateTimeList()           override;
    QList<int>          valueToIntOrIntList()           override;
    QList<double>       valueToDoubleOrDoubleList()     override;
    QList<QString>      valueToStringOrStringList()     override;

protected:

    SearchXml::Operator m_groupOperator = SearchXml::And;
    QString             m_groupCaption;
    SearchXml::Operator m_fieldOperator = SearchXml::And;
    QString             m_fieldName;
    SearchXml::Relation m_fieldRelation = SearchXml::Equal;
    QVariant            m_value;
    bool                m_readValue     = false;
};

} // namespace Digikam
