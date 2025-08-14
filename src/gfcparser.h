#pragma once
#include <QString>
#include <QVector>
#include <QHash>

struct GfcInstanceRef {
    int index = -1;
    QString cls;
    int pos = -1;
};

struct GfcParser {
    static QHash<QString,int> countClasses(const QString& text, QVector<GfcInstanceRef>* outRefs = nullptr);
    static int parseInstanceIndex(const QString& token);
};
