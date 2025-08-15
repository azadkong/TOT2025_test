#include "expressparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  #include <QStringConverter>
#endif

static QString stripInlineComment(const QString& s) {
    int i = s.indexOf(QStringLiteral("--"));
    if (i >= 0) return s.left(i);
    return s;
}

static QString trimSemicolonAndWhitespace(QString s) {
    s = s.trimmed();
    if (s.endsWith(';')) s.chop(1);
    return s.trimmed();
}

static QStringList parseParentsList(const QString& line) {
    // Capture inner content of SUBTYPE OF ( ... )
    QRegularExpression re(QStringLiteral("\\bSUBTYPE\\s+OF\\s*\\(\\s*([^\\)]*)\\)"),
                          QRegularExpression::CaseInsensitiveOption);
    auto m = re.match(line);
    QStringList out;
    if (m.hasMatch()) {
        QString inner = m.captured(1);
        for (QString part : inner.split(',', Qt::SkipEmptyParts)) {
            part = part.trimmed();
            // parent names are identifiers; strip trailing punctuation if any
            part = trimSemicolonAndWhitespace(part);
            if (!part.isEmpty())
                out.push_back(part);
        }
    }
    return out;
}

bool ExpressParser::parseFile(const QString& path, QString* err) {
    classes_.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (err) *err = QString("Cannot open: %1").arg(path);
        return false;
    }
    QTextStream ts(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Utf8);
#else
    ts.setCodec("UTF-8");
#endif

    QRegularExpression reEntityStart(R"(^\s*ENTITY\s+([A-Za-z_][A-Za-z0-9_]*)\b.*$)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reEnd(R"(^\s*END_ENTITY\s*;?\s*$)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reSubtypeLine(R"(^\s*SUBTYPE\s+OF\s*\(\s*([^)]+)\)\s*;?\s*$)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression reAttr(R"(^\s*([A-Za-z_][A-Za-z0-9_]*)\s*:\s*([A-Za-z_][A-Za-z0-9_]*)\s*;?\s*$)");

    bool inEntity = false;
    ExpClassInfo cur;

    while (!ts.atEnd()) {
        QString raw = ts.readLine();
        QString line = stripInlineComment(raw).trimmed();
        if (line.isEmpty())
            continue;

        if (!inEntity) {
            auto m = reEntityStart.match(line);
            if (m.hasMatch()) {
                inEntity = true;
                cur = ExpClassInfo{};
                cur.name = m.captured(1);
                cur.parent.clear();
                cur.parents.clear();
                cur.attributes.clear();

                // same-line SUBTYPE clause (possibly multiple parents)
                QStringList pl = parseParentsList(line);
                if (!pl.isEmpty()) {
                    cur.parents = pl;
                    cur.parent = pl.front();
                }
            }
            continue;
        }

        if (reEnd.match(line).hasMatch()) {
            classes_.insert(cur.name, cur);
            inEntity = false;
            continue;
        }

        if (reSubtypeLine.match(line).hasMatch()) {
            QStringList pl = parseParentsList(line);
            if (!pl.isEmpty()) {
                cur.parents = pl;
                cur.parent = pl.front();
            }
            continue;
        }

        auto am = reAttr.match(line);
        if (am.hasMatch()) {
            QString attr = am.captured(1) + QStringLiteral(" : ") + am.captured(2);
            cur.attributes.push_back(trimSemicolonAndWhitespace(attr));
            continue;
        }
        // ignore others
    }

    if (inEntity) {
        classes_.insert(cur.name, cur);
        if (err) err->clear();
        return true;
    }
    return true;
}

QHash<QString, QSet<QString>> ExpressParser::buildChildrenMap() const {
    QHash<QString, QSet<QString>> ch;

    QSet<QString> allKeys;
    for (auto it = classes_.cbegin(); it != classes_.cend(); ++it) {
        allKeys.insert(it.key());
        const auto& cls = it.value();
        for (const auto& p : cls.parents)
            allKeys.insert(p);
        if (!cls.parent.isEmpty())
            allKeys.insert(cls.parent);
    }
    for (const auto& k : allKeys)
        ch.insert(k, QSet<QString>{});

    for (auto it = classes_.cbegin(); it != classes_.cend(); ++it) {
        const auto& cls = it.value();
        if (!cls.parents.isEmpty()) {
            for (const auto& p : cls.parents)
                ch[p].insert(cls.name);
        } else if (!cls.parent.isEmpty()) {
            ch[cls.parent].insert(cls.name);
        }
    }
    return ch;
}
