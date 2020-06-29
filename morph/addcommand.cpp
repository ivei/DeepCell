#include <QPolygon>

#include "../srv/cellitem.h"

#include "addcommand.h"
#include "celllistmodel.h"

#pragma execution_character_set("utf-8")

AddCommand::AddCommand(const CellItem& item, CellListModel* model, QUndoCommand *parent)
    :QUndoCommand(parent), item(new CellItem(item)), model(model)
{

}

AddCommand::~AddCommand()
{

}

void AddCommand::undo()
{
    model->setDelete(item->uid(), true);
}

void AddCommand::redo()
{
    model->add(*item);
}
