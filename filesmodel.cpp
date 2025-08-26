#include <QDir>
#include <QFont>
#include <QBrush>
#include "filesmodel.h"

void FilesModel::read() {
  QString path = QDir::homePath() + "/Library/Application Support/It";
  //QDir dir(path);
  //QStringList files = dir.entryList(QStringList() << "*.cpp", QDir::Files);
  //foreach(QString filename, files) {
  //  qDebug() << filename;
  //}
  QString listpath = path + "/f_list.txt";
  QFile inputFile(listpath);
  if (inputFile.open(QIODevice::ReadOnly)) {
    QTextStream in(&inputFile);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList pieces = line.split("\\");
      files.append(FileEntry(
        pieces.size() > 1 ? pieces[0] : pieces[0].mid(1),
        pieces.size() > 1 ? pieces[1] : "",
        pieces.size() > 2
      ));
    }
    inputFile.close();
  }
}

void FilesModel::write() {

}

// https://doc.qt.io/qt-6/modelview.html#3-1-treeview

int FilesModel::rowCount(const QModelIndex &parent) const {
  return files.count();
}
int FilesModel::columnCount(const QModelIndex &parent) const {
  return 1;
}
QVariant FilesModel::data(const QModelIndex &index, int role) const {
  int row = index.row();
  const FileEntry &e = files[row];
  if (role == Qt::DisplayRole) {
    return e.name;
  } else if (role == Qt::FontRole) {
    if (e.file.isEmpty()) {
      QFont boldFont;
      boldFont.setBold(true);
      return boldFont;
    }
  } else if (role == Qt::BackgroundRole) {
    if (e.file.isEmpty()) {
      return QBrush(QColor::fromRgb(220, 220, 220));
    }
  }
  return QVariant();
}

bool FilesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (!checkIndex(index)) return false;
  int row = index.row();
  FileEntry &e = files[row];
  if (role == Qt::EditRole) {
    //if (!e.file.isEmpty()) return false;
    e.name = value.toString();
    return true;
  }
  return false;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

///////////////////////////////////////////////////////////////////////////////

TreeItem::TreeItem(const QStringList& data, TreeItem* parent) : itemData(data), _parentItem(parent) {
}
TreeItem::~TreeItem() {
  qDeleteAll(childItems);
}
void TreeItem::appendChild(TreeItem* item) {
  childItems.append(item);
}
void TreeItem::removeChild(int row) {
  if (row >= 0 && row < childItems.size()) {
    delete childItems.takeAt(row);
  }
}
TreeItem* TreeItem::child(int row) const {
  if (row < 0 || row >= childItems.size())
    return nullptr;
  return childItems.at(row);
}
int TreeItem::childCount() const {
    return childItems.count();
}
QVariant TreeItem::data(int column) const {
  if (column < 0 || column >= itemData.size())
    return QVariant();
  return itemData.at(column);
}
bool TreeItem::setData(int column, const QVariant& value) {
  if (column < 0 || column >= itemData.size())
    return false;
  itemData[column] = value.toString();
  return true;
}
int TreeItem::columnCount() const {
  return itemData.count();
}
TreeItem* TreeItem::parentItem() const {
  return _parentItem;
}
int TreeItem::row() const {
  if (_parentItem) {
    return _parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
  }
  return 0;
}

//////////////////////////////

TreeModel::TreeModel(const QStringList& headers, QObject* parent) : QAbstractItemModel(parent), headers(headers) {
  rootItem = new TreeItem(headers);
}
TreeModel::~TreeModel() {
  delete rootItem;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return item->data(index.column());
  case Qt::DecorationRole:
    if (index.column() == 0) {
      // Return folder/file icons based on whether item has children
      return item->childCount() > 0 ? QVariant("folder") : QVariant("file");
    }
    break;
  case Qt::FontRole:
    if (item->childCount() > 0) {
      QFont boldFont;
      boldFont.setBold(true);
      return boldFont;
    }
    break;
  }
  return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
  if (!index.isValid())
    return Qt::NoItemFlags;
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < headers.size())
    return headers.at(section);
  return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();
  TreeItem* parentItem = getItem(parent);
  TreeItem* childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) return QModelIndex();
  TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
  TreeItem* parentItem = childItem->parentItem();
  if (parentItem == rootItem) return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
  TreeItem* parentItem = getItem(parent);
  return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return headers.size();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role != Qt::EditRole) return false;
  TreeItem* item = getItem(index);
  bool result = item->setData(index.column(), value);
  if (result)
    emit dataChanged(index, index, {role});
  return result;
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const {
  if (index.isValid()) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item)return item;
  }
  return rootItem;
}

QModelIndex TreeModel::addChild(const QModelIndex& parent, const QStringList& data) {
  TreeItem* parentItem = getItem(parent);
  int row = parentItem->childCount();
  beginInsertRows(parent, row, row);
  TreeItem* newItem = new TreeItem(data, parentItem);
  parentItem->appendChild(newItem);
  endInsertRows();
  return index(row, 0, parent);
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
  TreeItem* parentItem = getItem(parent);
  beginInsertRows(parent, position, position + rows - 1);
  for (int row = 0; row < rows; ++row) {
    QStringList data;
    for (int col = 0; col < columnCount(); ++col) {
      data << QString("New Item");
    }
    TreeItem* item = new TreeItem(data, parentItem);
    parentItem->appendChild(item);
  }
  endInsertRows();
  return true;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
  TreeItem* parentItem = getItem(parent);
  beginRemoveRows(parent, position, position + rows - 1);
  for (int row = 0; row < rows; ++row) {
      parentItem->removeChild(position);
  }
  endRemoveRows();
  return true;
}

void selectAndExpandToItem(QTreeView* treeView, const QModelIndex& index) {
  if (!index.isValid()) return;
  // Expand all parents to make item visible
  QModelIndex parent = index.parent();
  while (parent.isValid()) {
      treeView->expand(parent);
      parent = parent.parent();
  }
  // Select the item
  treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
  treeView->scrollTo(index);
}

QModelIndex findItemByData(QAbstractItemModel* model, const QVariant& value, int column = 0, const QModelIndex& parent = QModelIndex()) {
  int rows = model->rowCount(parent);
  for (int row = 0; row < rows; ++row) {
    QModelIndex index = model->index(row, column, parent);
    if (model->data(index, Qt::DisplayRole) == value) {
      return index;
    }
    if (model->rowCount(index) > 0) { // recursively search children
      QModelIndex childResult = findItemByData(model, value, column, index);
      if (childResult.isValid()) {
        return childResult;
      }
    }
  }
  return QModelIndex(); // Not found
}

bool TreeModel::selectItem(QTreeView *treeView, const QString &value, int col) {
  QModelIndex index = findItemByData(this, value, col);
  if (index.isValid()) {
    selectAndExpandToItem(treeView, index);
    return true;
  }
  return false;
}

void TreeModel::addFolder(const QString &name) {
  _folder = addChild(_root, QStringList() << name);
}

void TreeModel::addEntry(const QString &name) {
  addChild(_folder, QStringList() << name);
}
