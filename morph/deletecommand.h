#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H
#include <QUndoCommand>

class CellItem;
class CellListModel;
class DeleteCommand : public QUndoCommand
{
public:
    DeleteCommand(const CellItem& item, CellListModel* model, QUndoCommand* parent=nullptr);
    ~DeleteCommand() override;

    void undo() override;
    void redo() override;

private:
    CellItem *item;
    CellListModel *model;
};

#endif // DELETECOMMAND_H
