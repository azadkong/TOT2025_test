#include <gtest/gtest.h>
#include <QString>
#include <QTemporaryFile>
#include <QFile>

#include "expressparser.h"

static QString writeTmpUtf8(const QString& body) {
    QTemporaryFile tf;
    tf.setAutoRemove(false);
    if (!tf.open()) return QString();
    tf.write(body.toUtf8());
    tf.close();
    return tf.fileName();
}

TEST(ExpressParser, EntitiesParentAndAttrs)
{
    const QString exp =
        "ENTITY A;\n"
        "  p1 : TYPE1;\n"
        "END_ENTITY;\n"
        "ENTITY B;\n"
        "  SUBTYPE OF (A);\n"
        "  b1 : TYPE2;\n"
        "END_ENTITY;\n";

    QString path = writeTmpUtf8(exp);
    ASSERT_FALSE(path.isEmpty());

    ExpressParser p;
    QString err;
    ASSERT_TRUE(p.parseFile(path, &err)) << err.toStdString();

    const auto& classes = p.classes();
    ASSERT_TRUE(classes.contains("A"));
    ASSERT_TRUE(classes.contains("B"));

    EXPECT_EQ(classes["A"].parent, "");
    ASSERT_EQ(classes["A"].attributes.size(), 1);
    EXPECT_EQ(classes["A"].attributes[0], "p1 : TYPE1");

    EXPECT_EQ(classes["B"].parent, "A");
    ASSERT_EQ(classes["B"].attributes.size(), 1);
    EXPECT_EQ(classes["B"].attributes[0], "b1 : TYPE2");
}



TEST(ExpressParser, AttributeTrimSemicolonAndWhitespace)
{
    const QString exp =
        "ENTITY X;\n"
        "  aa : TYPEA  ; \n"
        "  bb : TYPEB; \n"
        "END_ENTITY;\n";

    QString path = writeTmpUtf8(exp);
    ExpressParser p; QString err;
    ASSERT_TRUE(p.parseFile(path, &err)) << err.toStdString();

    const auto& cls = p.classes().value("X");
    ASSERT_EQ(cls.attributes.size(), 2);
    EXPECT_EQ(cls.attributes[0], "aa : TYPEA");
    EXPECT_EQ(cls.attributes[1], "bb : TYPEB");
}
