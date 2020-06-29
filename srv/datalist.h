#ifndef DATALIST_H
#define DATALIST_H
#include <QList>

template<typename T>
class DataList: public QList<T>
{
public:
    inline DataList(int total=-1) Q_DECL_NOTHROW: total(total) { }
    inline DataList(const DataList<T> &l) : QList<T>(l), total(l.total) { }
    inline DataList(std::initializer_list<T> args) : QList<T>(args), total(args.size()) { }
    DataList &operator=(const DataList<T> &other)
    { QList<T>::operator=(other); total = other.total; return *this; }
    inline int getTotal() const { return this->total; }
    inline bool hasMore() const { return total < 0 || size() < total; }
    inline void setTotal(int total) { this->total = total; }
private:
    int total;
};

#endif // DATALIST_H
