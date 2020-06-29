#include "../srv/cellitem.h"
#include "celllistmodel.h"
#include "deletecommand.h"

#pragma execution_character_set("utf-8")

DeleteCommand::DeleteCommand(const CellItem& item, CellListModel* model, QUndoCommand* parent)
    : QUndoCommand(parent), item(new CellItem(item)), model(model)
{

}

DeleteCommand::~DeleteCommand()
{
    delete item;
    model = nullptr;
}

void DeleteCommand::undo()
{
    model->setDelete(item->uid(), false);
}

void DeleteCommand::redo()
{
    model->setDelete(item->uid(), true);
}
