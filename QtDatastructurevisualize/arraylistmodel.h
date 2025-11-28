#pragma once
#pragma once
#include <QObject>
#include <vector>

class ArrayListModel : public QObject {
    Q_OBJECT
public:
    explicit ArrayListModel(QObject* parent = nullptr);
    void insertNode(int value);   // append at end
    void deleteNode(int value);
    void clearList();
    bool contains(int value) const;
    const std::vector<int>& data() const { return m_data; }
signals:
    void nodeInserted(int value);
    void nodeDeleted(int value);
    void listCleared();
private:
    std::vector<int> m_data;
};
