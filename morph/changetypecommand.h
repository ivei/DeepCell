#ifndef CHANGETYPECOMMAND_H
#define CHANGETYPECOMMAND_H
#include <QUndoCommand>

class CellItem;
class CellListModel;
class ChangeTypeCommand : public QUndoCommand
{
public:
    ChangeTypeCommand(const CellItem& item, CellListModel* model, int type, QUndoCommand* parent=nullptr);
    ~ChangeTypeCommand() override;
    void undo() override;
    void redo() override;

private:
    CellItem *item;
    CellListModel *model;
    int type;
};

#endif // CHANGETYPECOMMAND_H
