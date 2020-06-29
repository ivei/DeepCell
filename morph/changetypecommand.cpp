
#include "../srv/cellitem.h"

#include "changetypecommand.h"
#include "celllistmodel.h"

#pragma execution_character_set("utf-8")

ChangeTypeCommand::ChangeTypeCommand(const CellItem& item, CellListModel* model, int type, QUndoCommand* parent)
    : QUndoCommand(parent), item(new CellItem(item)), model(model), type(type)
{

}

ChangeTypeCommand::~ChangeTypeCommand()
{
    delete item;
    model = nullptr;
}

void ChangeTypeCommand::undo()
{
    model->changeType(item->uid(), item->type());
}

void ChangeTypeCommand::redo()
{
    model->changeType(item->uid(), type);
}
