#include "basescene.h"

BaseScene::BaseScene(QObject* parent) : QGraphicsScene(parent) {
    setSceneRect(0, 0, 1200, 600);
}

void BaseScene::onNodeInserted(int) {}
void BaseScene::onNodeDeleted(int) {}
void BaseScene::onListCleared() {}
void BaseScene::setStructureType(int) {}
