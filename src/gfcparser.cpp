#include "gfcparser.h"
#include <QRegularExpression>

QHash<QString,int> GfcParser::countClasses(const QString& text, QVector<GfcInstanceRef>* outRefs) {
    QHash<QString,int> counts;
    QRegularExpression re(R"(#\s*([0-9]+)\s*=\s*([A-Za-z_0-9]+)\s*\()");
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        int index = m.captured(1).toInt();
        QString cls = m.captured(2);
        counts[cls] += 1;
        if (outRefs) {
            GfcInstanceRef r; r.index = index; r.cls = cls; r.pos = m.capturedStart();
            outRefs->push_back(r);
        }
    }
    return counts;
}

int GfcParser::parseInstanceIndex(const QString& token) {
    QRegularExpression re(R"(^\s*#\s*([0-9]+)\s*$)");
    auto m = re.match(token);
    if (!m.hasMatch()) return -1;
    return m.captured(1).toInt();
}
