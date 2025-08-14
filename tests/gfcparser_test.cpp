#include <gtest/gtest.h>
#include <QString>
#include <QVector>
#include <QHash>

#include "gfcparser.h"

TEST(GfcParser, CountAndRefsBasic)
{
    const QString text =
        "#1=FOO(a,b);\n"
        "#2=BAR(x);\n"
        "  #3 = FOO( c );\n";

    QVector<GfcInstanceRef> refs;
    QHash<QString,int> counts = GfcParser::countClasses(text, &refs);

    ASSERT_EQ(refs.size(), 3);
    EXPECT_EQ(counts.value("FOO", 0), 2);
    EXPECT_EQ(counts.value("BAR", 0), 1);

    EXPECT_EQ(refs[0].index, 1);
    EXPECT_EQ(refs[0].cls, "FOO");
    EXPECT_GE(refs[0].pos, 0);
}

TEST(GfcParser, WhitespaceAndInvalidLines)
{
    const QString text =
        "#10   =   FOO(\n"
        ");\n"
        "NOT_AN_INSTANCE\n"
        "# 11 = FOO( );\n";

    QHash<QString,int> counts = GfcParser::countClasses(text, nullptr);
    EXPECT_EQ(counts.value("FOO", 0), 2);
    EXPECT_EQ(counts.contains("NOT_AN_INSTANCE"), false);
}

TEST(GfcParser, ParseInstanceIndex)
{
    EXPECT_EQ(GfcParser::parseInstanceIndex("#0"), 0);
    EXPECT_EQ(GfcParser::parseInstanceIndex("  #42 "), 42);
    EXPECT_EQ(GfcParser::parseInstanceIndex("#"), -1);
    EXPECT_EQ(GfcParser::parseInstanceIndex("42"), -1);
    EXPECT_EQ(GfcParser::parseInstanceIndex("#12a"), -1);
}
