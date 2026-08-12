#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

#find_package(Protobuf)
#set_package_properties(Protobuf PROPERTIES TYPE OPTIONAL PURPOSE "Parsing of OSM PBF files.")
#
#if(Protobuf_FOUND AND Protobuf_PROTOC_EXECUTABLE)
#
#    include_directories(${Protobuf_INCLUDE_DIRS})
#
#    PROTOBUF_GENERATE_CPP(pbf_srcs pbf_hdrs
#                          pbf/fileformat.proto
#                          pbf/osmformat.proto
#    )
#
#    set(OsmPlugin_ExtraLibs ${Protobuf_LIBRARIES})
#    add_definitions(-DHAVE_PROTOBUF)
#
#    if(MSVC)
#        add_definitions(-DPROTOBUF_USE_DLLS)
#    endif()
#
#endif()

set(osm_writers_SRCS
    writers/OsmObjectAttributeWriter.cpp
    writers/OsmNodeTagWriter.cpp
    writers/OsmWayTagWriter.cpp
    writers/OsmRelationTagWriter.cpp
    writers/OsmTagWriter.cpp
    writers/OsmTagTagWriter.cpp
)

set(osm_translators_SRCS
    translators/OsmDocumentTagTranslator.cpp
    translators/O5mWriter.cpp
    translators/OsmConverter.cpp
)

set(osm_SRCS
    OsmParser.cpp
    OsmPlugin.cpp
    OsmRunner.cpp
    OsmNode.cpp
    OsmWay.cpp
    OsmRelation.cpp
    OsmElementDictionary.cpp
    OsmPbfParser.cpp
    o5mreader/o5mreader.cpp
    ${pbf_srcs}
    ${osm_writers_SRCS}
    ${osm_translators_SRCS}
)

marble_add_plugin(NAME    OsmPlugin
                  SOURCES ${osm_SRCS}
#                  DEPENDS ${OsmPlugin_ExtraLibs}
)
