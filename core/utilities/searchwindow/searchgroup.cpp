/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2026 by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchgroup_p.h"

namespace Digikam
{

SearchGroup::SearchGroup(SearchView* const parent)
    : AbstractSearchGroupContainer(parent),
      m_view                      (parent)
{
}

void SearchGroup::setup(Type type)
{
    m_groupType = type;

    m_layout    = new QVBoxLayout;
    m_layout->setContentsMargins(QMargins());
    m_layout->setSpacing(0);

    m_label     = new SearchGroupLabel(m_view, m_groupType, this);
    m_layout->addWidget(m_label);

    connect(m_label, SIGNAL(removeClicked()),
            this, SIGNAL(removeRequested()));

    SearchFieldGroup*      group = nullptr;
    SearchFieldGroupLabel* label = nullptr;

    // To prevent Cppcheck warnings.
    (void)group;
    (void)label;

    // ----- //

    group    = new SearchFieldGroup(this);
    group->addField(SearchField::createField(QLatin1String("keyword"), group));
    m_fieldGroups << group;
    m_layout->addWidget(group);

    // this group has no label. Need to show, else it is hidden forever

    group->setFieldsVisible(true);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("File, Album, Tags"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("albumid"),         group));
    group->addField(SearchField::createField(QLatin1String("albumname"),       group));
    group->addField(SearchField::createField(QLatin1String("albumcollection"), group));
    group->addField(SearchField::createField(QLatin1String("tagid"),           group));
    group->addField(SearchField::createField(QLatin1String("tagname"),         group));
    group->addField(SearchField::createField(QLatin1String("nottagged"),       group));
    group->addField(SearchField::createField(QLatin1String("filename"),        group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Item Properties"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("creationdate"),     group));
    group->addField(SearchField::createField(QLatin1String("creationtime"),     group));
    group->addField(SearchField::createField(QLatin1String("rating"),           group));
    group->addField(SearchField::createField(QLatin1String("labels"),           group));
    group->addField(SearchField::createField(QLatin1String("dimension"),        group));
    group->addField(SearchField::createField(QLatin1String("pageorientation"),  group));
    group->addField(SearchField::createField(QLatin1String("width"),            group));
    group->addField(SearchField::createField(QLatin1String("height"),           group));
    group->addField(SearchField::createField(QLatin1String("aspectratioimg"),   group));
    group->addField(SearchField::createField(QLatin1String("pixelsize"),        group));
    group->addField(SearchField::createField(QLatin1String("format"),           group));
    group->addField(SearchField::createField(QLatin1String("colordepth"),       group));
    group->addField(SearchField::createField(QLatin1String("colormodel"),       group));
    group->addField(SearchField::createField(QLatin1String("modificationdate"), group));
    group->addField(SearchField::createField(QLatin1String("digitizationdate"), group));
    group->addField(SearchField::createField(QLatin1String("modifiedrecent"), group));
    group->addField(SearchField::createField(QLatin1String("filesize"),         group));
    group->addField(SearchField::createField(QLatin1String("bytesize"),         group));
    group->addField(SearchField::createField(QLatin1String("monthday"),         group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Audio/Video Properties"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("videoaspectratio"),      group));
    group->addField(SearchField::createField(QLatin1String("videoduration"),         group));
    group->addField(SearchField::createField(QLatin1String("videoframerate"),        group));
    group->addField(SearchField::createField(QLatin1String("videocodec"),            group));
    group->addField(SearchField::createField(QLatin1String("videoaudiobitrate"),     group));
    group->addField(SearchField::createField(QLatin1String("videoaudiochanneltype"), group));
    group->addField(SearchField::createField(QLatin1String("videoaudioCodec"),       group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Caption, Comment, Title"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("creator"),       group));
    group->addField(SearchField::createField(QLatin1String("comment"),       group));
    group->addField(SearchField::createField(QLatin1String("commentauthor"), group));
    group->addField(SearchField::createField(QLatin1String("headline"),      group));
    group->addField(SearchField::createField(QLatin1String("title"),         group));
    group->addField(SearchField::createField(QLatin1String("emptytext"),     group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Photograph Information"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("make"),                         group));
    group->addField(SearchField::createField(QLatin1String("model"),                        group));
    group->addField(SearchField::createField(QLatin1String("lenses"),                       group));
    group->addField(SearchField::createField(QLatin1String("nocamera"),                     group));
    group->addField(SearchField::createField(QLatin1String("aperture"),                     group));
    group->addField(SearchField::createField(QLatin1String("focallength"),                  group));
    group->addField(SearchField::createField(QLatin1String("focallength35"),                group));
    group->addField(SearchField::createField(QLatin1String("exposuretime"),                 group));
    group->addField(SearchField::createField(QLatin1String("exposureprogram"),              group));
    group->addField(SearchField::createField(QLatin1String("exposuremode"),                 group));
    group->addField(SearchField::createField(QLatin1String("sensitivity"),                  group));
    group->addField(SearchField::createField(QLatin1String("orientation"),                  group));
    group->addField(SearchField::createField(QLatin1String("flashmode"),                    group));
    group->addField(SearchField::createField(QLatin1String("whitebalance"),                 group));
    group->addField(SearchField::createField(QLatin1String("whitebalancecolortemperature"), group));
    group->addField(SearchField::createField(QLatin1String("meteringmode"),                 group));
    group->addField(SearchField::createField(QLatin1String("subjectdistance"),              group));
    group->addField(SearchField::createField(QLatin1String("subjectdistancecategory"),      group));
    group->addField(SearchField::createField(QLatin1String("nocapturesettings"),                     group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Geographic position"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

/*
    group->addField(SearchField::createField("latitude", group));
    group->addField(SearchField::createField("longitude", group));
*/
    group->addField(SearchField::createField(QLatin1String("altitude"),      group));
    group->addField(SearchField::createField(QLatin1String("nogps"),         group));
    group->addField(SearchField::createField(QLatin1String("country"),       group));
    group->addField(SearchField::createField(QLatin1String("provinceState"), group));
    group->addField(SearchField::createField(QLatin1String("city"),          group));
    group->addField(SearchField::createField(QLatin1String("location"),      group));
    group->addField(SearchField::createField(QLatin1String("noplaces"),      group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    label = new SearchFieldGroupLabel(this);
    label->setTitle(i18n("Face regions"));
    group = new SearchFieldGroup(this);
    group->setLabel(label);

    group->addField(SearchField::createField(QLatin1String("faceregionscount"), group));
    group->addField(SearchField::createField(QLatin1String("confirmedfaces"),   group));
    group->addField(SearchField::createField(QLatin1String("nofaceregions"),    group));

    m_fieldLabels << label;
    m_fieldGroups << group;
    m_layout->addWidget(label);
    m_layout->addWidget(group);

    // ----- //

    // prepare subgroup layout

    QHBoxLayout* const indentLayout = new QHBoxLayout;
    indentLayout->setContentsMargins(QMargins());
    indentLayout->setSpacing(0);

    QStyleOption option;
    option.initFrom(this);
    int indent = 5 * style()->pixelMetric(QStyle::PM_LayoutLeftMargin, &option, this);
    indent     = qMax(indent, 20);
    indentLayout->addSpacing(indent);

    m_subgroupLayout = new QVBoxLayout;
    m_subgroupLayout->setContentsMargins(QMargins());
    m_subgroupLayout->setSpacing(0);

    indentLayout->addLayout(m_subgroupLayout);

    m_layout->addLayout(indentLayout);

    // ----- //

    m_layout->addStretch(1);
    setLayout(m_layout);

    // ----- //

    // initialization as empty group

    reset();
}

void SearchGroup::read(SearchXmlCachingReader& reader)
{
    reset();

    m_label->setGroupOperator(reader.groupOperator());
    m_label->setDefaultFieldOperator(reader.defaultFieldOperator());

    startReadingGroups(reader);

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
        {
            break;
        }

        // subgroup

        if (reader.isGroupElement())
        {
            readGroup(reader);
        }

        if (reader.isFieldElement())
        {
            QString name = reader.fieldName();

            SearchField* field           = nullptr;
            SearchFieldGroup* fieldGroup = nullptr;

            for (SearchFieldGroup* const fg : std::as_const(m_fieldGroups))
            {
                fieldGroup = fg;

                if (fieldGroup)
                {
                    if ((field = fieldGroup->fieldForName(name)))
                    {
                        // cppcheck-suppress useStlAlgorithm
                        break;
                    }
                }
            }

            if (field)
            {
                field->read(reader);
                fieldGroup->markField(field);
                fieldGroup->setFieldsVisible(true);
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unhandled search field in XML with field name" << name;
                reader.readToEndOfElement();
            }
        }
    }

    finishReadingGroups();
}

SearchGroup* SearchGroup::createSearchGroup()
{
    // create a sub group - view is the same

    SearchGroup* const group = new SearchGroup(m_view);
    group->setup(SearchGroup::ChainGroup);

    return group;
}

void SearchGroup::addGroupToLayout(SearchGroup* group)
{
    // insert in front of the stretch

    m_subgroupLayout->addWidget(group);
}

void SearchGroup::write(SearchXmlWriter& writer)
{
    writer.writeGroup();
    writer.setGroupOperator(m_label->groupOperator());
    writer.setDefaultFieldOperator(m_label->defaultFieldOperator());

    for (SearchFieldGroup* const fieldGroup : std::as_const(m_fieldGroups))
    {
        fieldGroup->write(writer);
    }

    // take care for subgroups

    writeGroups(writer);

    writer.finishGroup();
}

void SearchGroup::reset()
{
    for (SearchFieldGroup* const fieldGroup : std::as_const(m_fieldGroups))
    {
        fieldGroup->reset();
    }

    m_label->setGroupOperator(SearchXml::standardGroupOperator());
    m_label->setDefaultFieldOperator(SearchXml::standardFieldOperator());
}

SearchGroup::Type SearchGroup::groupType() const
{
    return m_groupType;
}

QList<QRect> SearchGroup::startupAnimationArea() const
{
    QList<QRect> rects;

    // from subgroups;

    rects += startupAnimationAreaOfGroups();

    // field groups

    for (const SearchFieldGroup* const fieldGroup : std::as_const(m_fieldGroups))
    {
        // cppcheck-suppress useStlAlgorithm
        rects += fieldGroup->areaOfMarkedFields();
    }

    // adjust position relative to parent

    for (QList<QRect>::iterator it = rects.begin() ; it != rects.end() ; ++it)
    {
        (*it).translate(pos());
    }

    return rects;
}

} // namespace Digikam

#include "moc_searchgroup.cpp"
