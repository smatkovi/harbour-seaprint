// The IPP tag numbers the QML writes into a job attribute.
//
// Upstream hands the QML IppMsg itself, which carries the enum as a Q_GADGET.
// QtDeclarative can only take enums off a QObject, and IppMsg must stay a
// value (it travels through queued connections by value), so the tags the QML
// names are repeated on a QObject of their own, registered under the name
// "IppMsg" so that qml/ keeps its spelling. The static assertions below keep
// the two lists from drifting apart.
#ifndef IPPTAGS_H
#define IPPTAGS_H

#include <QObject>
#include <src/ippmsg.h>

class IppTags : public QObject
{
    Q_OBJECT
    Q_ENUMS(IppTag)

public:
    enum IppTag {
        Integer             = 0x21,
        Enum                = 0x23,
        Resolution          = 0x32,
        IntegerRange        = 0x33,
        BeginCollection     = 0x34,
        Keyword             = 0x44,
        MimeMediaType       = 0x49
    };
};

// moc swallows whatever declaration follows Q_ENUMS, so these live outside the
// class rather than after the enum.
static_assert(int(IppTags::Integer)         == int(IppMsg::Integer),         "IppTag drift");
static_assert(int(IppTags::Enum)            == int(IppMsg::Enum),            "IppTag drift");
static_assert(int(IppTags::Resolution)      == int(IppMsg::Resolution),      "IppTag drift");
static_assert(int(IppTags::IntegerRange)    == int(IppMsg::IntegerRange),    "IppTag drift");
static_assert(int(IppTags::BeginCollection) == int(IppMsg::BeginCollection), "IppTag drift");
static_assert(int(IppTags::Keyword)         == int(IppMsg::Keyword),         "IppTag drift");
static_assert(int(IppTags::MimeMediaType)   == int(IppMsg::MimeMediaType),   "IppTag drift");

#endif // IPPTAGS_H
