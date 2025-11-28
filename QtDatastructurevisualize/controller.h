#pragma once
#include <QObject>

class BaseScene;
class ArrayListModel;
class LinkedListModel;
class StackModel;

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(BaseScene* scene, QObject* parent = nullptr);

public slots:
    void onInsertRequested(const QString& value);
    void onRemoveRequested(const QString& value);
    void onFindRequested(const QString& value);
    void onResetRequested();
    void onStructureChanged(int idx);

private:
    BaseScene* m_scene = nullptr;
    // one model active at a time
    ArrayListModel* m_arrayModel = nullptr;
    LinkedListModel* m_linkedModel = nullptr;
    StackModel* m_stackModel = nullptr;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2 } m_struct = LINKED;

    void ensureModels();
    int toIntOrWarn(const QString& s);
};
