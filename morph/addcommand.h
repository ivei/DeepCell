#ifndef ADDCOMMAND_H
#define ADDCOMMAND_H

#include <QUndoCommand>

class CellListModel;
class CellItem;
class AddCommand : public QUndoCommand
{
public:
    AddCommand(const CellItem &item, CellListModel* model, QUndoCommand *parent = nullptr);
    ~AddCommand()  override;

    void undo() override;
    void redo() override;
private:
    CellItem *item;
    CellListModel *model;
};

#endif // ADDCOMMAND_H
