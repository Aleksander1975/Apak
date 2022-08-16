

#include "treeitem.h"

#include <QStringList>

//! [0]
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
    
    ItemInfo inf;
    for(int i = 0; i < itemData.count(); i++)
      itemInfo.append(inf);
    
}
//! [0]

//! [1]
TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}
//! [1]

//! [2]
TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}
//! [2]

//! [3]
int TreeItem::childCount() const
{
    return childItems.count();
}
//! [3]

//! [4]
int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

ItemInfo TreeItem::info(int column) const
{
    return itemInfo.value(column);
}

TreeItem* TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return nullptr;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }

    return childItems.at(position);
}
//! [7]

//! [8]
bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
    {
        itemData.insert(position, QVariant());
        ItemInfo inf;
        itemInfo.insert(position, inf);
    }

    foreach (TreeItem *child, childItems)
        child->insertColumns(position, columns);

    return true;
}
//! [8]

//! [9]
TreeItem *TreeItem::parent()
{
    return parentItem;
}
//! [9]

//! [10]
bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
//! [10]

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
    {
        itemData.remove(position);
        itemInfo.remove(position);
    }

    foreach (TreeItem *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}

bool TreeItem::setInfo(int column, ItemInfo info)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemInfo[column] = info;

    return true;
}
