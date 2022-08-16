/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

enum ItemTypes
{
  itDevices = 65537,
  itDevice,
  itDeviceId,
  itDeviceIndex,
  itDeviceName,
  itDeviceIfcType,
  itDeviceIfcProtocol,
  itDeviceIfcName,
  itDeviceDataType,
  itDeviceDataLength,
  itDeviceParams,
  itDeviceDescription,
  itSignals,
  itSignal,
  itSignalIndex,
  itSignalName,
  itSignalCobId,
  itSignalDataLenght,
  itSignalOffset,
  itSignalTypeDiscrete,
  itSignalTypeAnalog,
  itSignalDescription,
  itSignalStorageLink,
  itStorage,
  itStorageId,
  itStorageName,
  itStorageHost,
  itStoragePort,
  itStorageLogin,
  itStoragePass,
  itStorageDBName,
  itStorageTableName,
    itDevicesRoot,
  itDevicesRootIcon,
    itStoragesRoot,
  itStoragesRootIcon,
  itUndefined,
    itDeviceWithNoSignals,
    itStorageWithNoSignals,
  itStandRoot,
  itStandRootIcon,
  itAutostart,
  itConfig,
  itStandInfo,
  itLogger
};

struct ItemInfo
{
  ItemInfo() { }
  ItemInfo(int type, QString fieldName, bool checkable = false, bool checked = false)
  { 
    this->type = type;
    this->fieldName = fieldName; 
    is_checkable = checkable;
    is_checked = checked;
  }
  int type = itUndefined;
  QString fieldName = "";
  
  bool is_checkable = false;
  bool is_checked = false;
};
    

//! [0]
class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    ItemInfo info(int column) const;
//    int type(int column) const;
//    QString columnName(int column) const;
    
    TreeItem *insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    
    bool setData(int column, const QVariant &value);
    bool setInfo(int column, ItemInfo info);
    
    void setCheckable(int column, bool checkable) { itemInfo[column].is_checkable = checkable; }
    void setChecked(int column, bool checked) { itemInfo[column].is_checked = checked; }
    
    bool checkable(int column) { return itemInfo[column].is_checkable; }
    bool checked(int column) { return itemInfo[column].is_checked; }
    
    int index = -1;
    int parent_index = -1;
    bool is_main_row = false;
    int item_state = -1;
    ItemTypes item_type;// = -1;
    
//    bool setType(int column, int type, QString columnName);
    
private:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
    QVector<ItemInfo> itemInfo = QVector<ItemInfo>();
    
//    QVector<QString> itemColumnNames = QVector<QString>();
//    QVector<int> itemTypes = QVector<int>();
    TreeItem *parentItem;
};
//! [0]

#endif // TREEITEM_H
