#include "expressparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

static QString trimSemicolon(QString s) {
    if (s.endsWith(';')) s.chop(1);
    return s.trimmed();
}

bool ExpressParser::parseFile(const QString& path, QString* err) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        if (err) *err = QString("Cannot open: %1").arg(path);
        return false;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
#else
    QTextStream ts(&f);
    ts.setCodec("utf-8");
#endif

    QRegularExpression reEntity(R"(^\s*ENTITY\s+([A-Za-z_][A-Za-z_0-9]*)\s*;)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reEnd(R"(^\s*END_ENTITY\s*;)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reSubtype(R"(^\s*SUBTYPE\s+OF\s*\(\s*([A-Za-z_][A-Za-z_0-9]*)\s*\)\s*;)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reAttr(R"(^\s*([A-Za-z_][A-Za-z_0-9\s]*)\s*:\s*([^;]+)\s*;\s*$)");

    bool in = false;
    ExpClassInfo cur;
    while (!ts.atEnd()) {
        const QString line = ts.readLine();
        if (!in) {
            auto m = reEntity.match(line);
            if (m.hasMatch()) {
                in = true;
                cur = ExpClassInfo{};
                cur.name = m.captured(1);
            }
            continue;
        }
        if (reEnd.match(line).hasMatch()) {
            classes_.insert(cur.name, cur);
            in = false;
            continue;
        }
        auto sm = reSubtype.match(line);
        if (sm.hasMatch()) {
            cur.parent = sm.captured(1);
            continue;
        }
        auto am = reAttr.match(line);
        if (am.hasMatch()) {
            cur.attributes.push_back(trimSemicolon(line).trimmed());
            continue;
        }
    }
    return true;
}

QHash<QString, QSet<QString>> ExpressParser::buildChildrenMap() const {
    QHash<QString, QSet<QString>> ch;
    for (const auto& k : classes_.keys()) ch[k] = QSet<QString>{};
    for (const auto& cls : classes_) {
        if (!cls.parent.isEmpty())
            ch[cls.parent].insert(cls.name);
    }
    return ch;
}
