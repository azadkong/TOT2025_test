#pragma once
#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>

struct ExpClassInfo {
    QString name;
    QString parent;           // first parent for backward-compat tests
    QStringList parents;      // all parents (multiple allowed)
    QStringList attributes;
};

class ExpressParser {
public:
    bool parseFile(const QString& path, QString* err);
    const QHash<QString, ExpClassInfo>& classes() const { return classes_; }
    QHash<QString, QSet<QString>> buildChildrenMap() const;

private:
    QHash<QString, ExpClassInfo> classes_;
};
