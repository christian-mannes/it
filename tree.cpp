#include "tree.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

///////////////////////////////////////////////////////////////////////////////
// TreeItem Implementation

TreeItem::TreeItem(const QString& displayName, ItemType type, void* data)
    : m_displayName(displayName), m_type(type), m_data(data), m_parent(nullptr) {
  readOnly = false;
}

TreeItem::~TreeItem() {
  qDeleteAll(m_children);
}

TreeItem* TreeItem::child(int index) const {
  if (index < 0 || index >= m_children.size())
    return nullptr;
  return m_children.at(index);
}

int TreeItem::indexInParent() const {
  if (!m_parent) return 0;
  return m_parent->m_children.indexOf(const_cast<TreeItem*>(this));
}

void TreeItem::appendChild(TreeItem* child) {
  insertChild(m_children.size(), child);
}

void TreeItem::insertChild(int index, TreeItem* child) {
  if (!child || child->m_parent == this) return;
  if (child->m_parent) {
    child->m_parent->removeChild(child);
  }
  index = qBound(0, index, m_children.size());
  m_children.insert(index, child);
  child->m_parent = this;
}

void TreeItem::removeChild(TreeItem* child) {
  if (!child || child->m_parent != this) return;
  m_children.removeOne(child);
  child->m_parent = nullptr;
}

void TreeItem::removeChild(int index) {
  if (index < 0 || index >= m_children.size()) return;
  TreeItem* child = m_children.takeAt(index);
  child->m_parent = nullptr;
}

void TreeItem::moveTo(TreeItem* newParent, int newIndex) {
  if (!newParent || newParent == m_parent) return;
  // Only folders can contain children
  if (!newParent->isFolder() && newParent->m_parent != nullptr) return;
  TreeItem* oldParent = m_parent;
  if (oldParent) {
    oldParent->removeChild(this);
  }
  newParent->insertChild(newIndex, this);
}

TreeItem* TreeItem::findChild(const QString& displayName, bool recursive) const {
  for (TreeItem* child : m_children) {
    if (child->displayName() == displayName) {
      return child;
    }
    if (recursive && child->isFolder()) {
      TreeItem* found = child->findChild(displayName, true);
      if (found) {
        return found;
      }
    }
  }
  return nullptr;
}

QList<TreeItem*> TreeItem::findAll(std::function<bool(TreeItem*)> predicate, bool recursive) const {
  QList<TreeItem*> results;
  for (TreeItem* child : m_children) {
    if (predicate(child)) {
      results.append(child);
    }
    if (recursive && child->isFolder()) {
      results.append(child->findAll(predicate, true));
    }
  }
  return results;
}

QString TreeItem::path(const QString& separator) const {
  if (!m_parent) {
    return m_displayName;
  }
  return m_parent->path(separator) + separator + m_displayName;
}

TreeItem* TreeItem::itemAtPath(const QString& path, const QString& separator) const {
  QStringList parts = path.split(separator, Qt::SkipEmptyParts);
  if (parts.isEmpty()) {
    return const_cast<TreeItem*>(this);
  }
  QString firstPart = parts.takeFirst();
  TreeItem* child = findChild(firstPart, false);
  if (!child) {
    return nullptr;
  }
  if (parts.isEmpty()) {
    return child;
  }
  return child->itemAtPath(parts.join(separator), separator);
}

///////////////////////////////////////////////////////////////////////////////
// TreeModel Implementation

const QString TreeModel::MIME_TYPE = "application/x-hierarchical-item-list";

TreeModel::TreeModel(QObject* parent) : QAbstractItemModel(parent), m_rootItem(nullptr) {
  // Create a default root item
  m_rootItem = new TreeItem("__root__", TreeItem::Folder);
}

TreeModel::~TreeModel() {
  delete m_rootItem;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();
  TreeItem* parentItem = itemFromIndex(parent);
  TreeItem* childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) return QModelIndex();
  TreeItem* childItem = itemFromIndex(index);
  TreeItem* parentItem = childItem->parent();
  if (parentItem == m_rootItem)
    return QModelIndex();
  return createIndex(parentItem->indexInParent(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
  TreeItem* parentItem = itemFromIndex(parent);
  return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  return 1; // Single column for display name
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();
  TreeItem* item = itemFromIndex(index);

  switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item->displayName();
    case Qt::DecorationRole:
        //return item->isFolder() ? QIcon(":/icons/folder.png") : QIcon(":/icons/document.png");
        return item->isFolder() ? QVariant("folder") : QVariant("file");
    case Qt::FontRole:
      if (item->isFolder()) {
        QFont boldFont;
        boldFont.setBold(true);
        return boldFont;
      } else if (item->readOnly) {
        QFont italicFont;
        italicFont.setItalic(true);
        return italicFont;
      }
      break;
    case Qt::UserRole:
        return QVariant::fromValue(static_cast<void*>(item));
    default:
        break;
  }
  return QVariant();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || role != Qt::EditRole) return false;
  TreeItem* item = itemFromIndex(index);
  QString oldName = item->displayName();
  QString newName = value.toString();
  if (oldName == newName) return false;
  item->setDisplayName(newName);
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  emit itemRenamed(item, oldName, newName);
  return true;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
  if (!index.isValid())
    return Qt::ItemIsDropEnabled; // Root can accept drops
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
  TreeItem* item = itemFromIndex(index);
  if (item->isFolder()) {
    flags |= Qt::ItemIsDropEnabled;
  }
  return flags;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    return "Name";
  return QVariant();
}

Qt::DropActions TreeModel::supportedDropActions() const {
  return Qt::MoveAction;
}

QStringList TreeModel::mimeTypes() const {
  return QStringList() << MIME_TYPE;
}

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) return nullptr;

  QMimeData* mimeData = new QMimeData();
  QJsonObject jsonData;

  // For simplicity, we'll just store the pointer value
  // In a real application, you might want a more robust serialization
  TreeItem* item = itemFromIndex(indexes.first());
  jsonData["itemPtr"] = QString::number(reinterpret_cast<qintptr>(item));

  QJsonDocument doc(jsonData);
  mimeData->setData(MIME_TYPE, doc.toJson());

  return mimeData;
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  Q_UNUSED(column)
  if (!data->hasFormat(MIME_TYPE) || action != Qt::MoveAction) return false;

  QJsonDocument doc = QJsonDocument::fromJson(data->data(MIME_TYPE));
  QJsonObject jsonData = doc.object();

  bool ok;
  qintptr itemPtr = jsonData["itemPtr"].toString().toLongLong(&ok);
  if (!ok) return false;

  TreeItem* draggedItem = reinterpret_cast<TreeItem*>(itemPtr);
  TreeItem* targetParent = itemFromIndex(parent);

  if (!draggedItem || !targetParent) return false;

  // Prevent dropping on self or descendant
  TreeItem* checkParent = targetParent;
  while (checkParent) {
    if (checkParent == draggedItem) return false;
    checkParent = checkParent->parent();
  }
  // Only folders can accept drops (except root)
  if (!targetParent->isFolder() && targetParent != m_rootItem) return false;

  TreeItem* oldParent = draggedItem->parent();
  // Calculate target row
  int targetRow = row;
  if (targetRow == -1) {
    targetRow = targetParent->childCount();
  }
  // Perform the move
  beginMoveRows(indexFromItem(oldParent), draggedItem->indexInParent(), draggedItem->indexInParent(), parent, targetRow);
  draggedItem->moveTo(targetParent, targetRow);
  endMoveRows();

  emit itemMoved(draggedItem, oldParent, targetParent);
  return true;
}

void TreeModel::setRootItem(TreeItem* root) {
  beginResetModel();
  delete m_rootItem;
  m_rootItem = root;
  endResetModel();
}

TreeItem* TreeModel::itemFromIndex(const QModelIndex& index) const {
  if (index.isValid()) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item) return item;
  }
  return m_rootItem;
}

QModelIndex TreeModel::indexFromItem(TreeItem* item) const {
  if (!item || item == m_rootItem)
    return QModelIndex();
  return createIndex(item->indexInParent(), 0, item);
}

QModelIndex TreeModel::addFolder(const QModelIndex& parent, const QString& name) {
  TreeItem* parentItem = itemFromIndex(parent);
  int row = parentItem->childCount();
  beginInsertRows(parent, row, row);
  TreeItem* newFolder = new TreeItem(name, TreeItem::Folder);
  parentItem->appendChild(newFolder);
  endInsertRows();
  emit itemAdded(newFolder, parentItem);
  return indexFromItem(newFolder);
}

QModelIndex TreeModel::addItem(const QModelIndex& parent, const QString& name, void* data) {
  TreeItem* parentItem = itemFromIndex(parent);
  int row = parentItem->childCount();
  beginInsertRows(parent, row, row);
  TreeItem* newItem = new TreeItem(name, TreeItem::Item, data);
  parentItem->appendChild(newItem);
  endInsertRows();
  emit itemAdded(newItem, parentItem);
  return indexFromItem(newItem);
}

void TreeModel::removeItem(const QModelIndex& index) {
  if (!index.isValid()) return;
  TreeItem* item = itemFromIndex(index);
  TreeItem* parentItem = item->parent();
  if (!parentItem) return;

  int row = item->indexInParent();
  beginRemoveRows(indexFromItem(parentItem), row, row);
  parentItem->removeChild(item);
  endRemoveRows();

  emit itemRemoved(item, parentItem);
  delete item;
}

TreeItem* TreeModel::findItemByName(const QString& name, bool recursive) const {
  return m_rootItem->findChild(name, recursive);
}

QModelIndex TreeModel::findIndexByName(const QString& name, bool recursive) const {
  TreeItem* item = findItemByName(name, recursive);
  return item ? indexFromItem(item) : QModelIndex();
}

bool TreeModel::selectItemByName(const QString& name, bool recursive) {
  QModelIndex index = findIndexByName(name, recursive);
  if (index.isValid()) {
    // This method only finds the index - actual selection should be done by the view
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// TreeView Implementation

TreeView::TreeView(QWidget* parent) : QTreeView(parent), m_treeModel(nullptr), m_contextMenuEnabled(true) {
  setupContextMenu();

  setDragDropMode(QAbstractItemView::InternalMove);
  setDefaultDropAction(Qt::MoveAction);
  setDropIndicatorShown(true);
  setDragEnabled(true);
  setAcceptDrops(true);
  preferredWidth = 200;
  preferredHeight = 200;
}

void TreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const {
  QRect newRect = rect;
  newRect.moveLeft(20);
  QTreeView::drawBranches(painter, newRect, index);
}
void TreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QTreeView::drawRow(painter, option, index);
}

void TreeView::setTreeModel(TreeModel* model) {
  m_treeModel = model;
  setModel(model);
}

TreeModel* TreeView::treeModel() const {
  return m_treeModel;
}

TreeItem* TreeView::currentItem() const {
  if (!m_treeModel) return nullptr;
  return m_treeModel->itemFromIndex(currentIndex());
}

QList<TreeItem*> TreeView::selectedItems() const {
  QList<TreeItem*> items;
  if (!m_treeModel) return items;
  QModelIndexList indexes = selectionModel()->selectedIndexes();
  for (const QModelIndex& index : indexes) {
    items.append(m_treeModel->itemFromIndex(index));
  }
  return items;
}

void TreeView::setDragDropEnabled(bool enabled) {
  setDragEnabled(enabled);
  setAcceptDrops(enabled);
  setDragDropMode(enabled ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
}

void TreeView::addFolder_() { addFolder("New Folder"); }
void TreeView::addFolder(const QString &name) {
  if (!m_treeModel) return;

  QModelIndex parent = currentIndex();
  TreeItem* parentItem = m_treeModel->itemFromIndex(parent);

  if (!parentItem->isFolder() && parentItem->parent()) {
    parent = m_treeModel->indexFromItem(parentItem->parent());
  }

  QModelIndex newIndex = m_treeModel->addFolder(parent, name);
  setCurrentIndex(newIndex);
  edit(newIndex);
}

void TreeView::addItem_() { addItem("New Item"); }
void TreeView::addItem(const QString &name) {
  if (!m_treeModel) return;
  QModelIndex parent = currentIndex();
  TreeItem* parentItem = m_treeModel->itemFromIndex(parent);
  if (!parentItem->isFolder() && parentItem->parent()) {
    parent = m_treeModel->indexFromItem(parentItem->parent());
  }
  QModelIndex newIndex = m_treeModel->addItem(parent, name);
  setCurrentIndex(newIndex);
  edit(newIndex);
}

void TreeView::renameItem() {
  QModelIndex current = currentIndex();
  if (!current.isValid() || !m_treeModel) return;
  TreeItem* item = m_treeModel->itemFromIndex(current);
  if (item->readOnly) return;
  edit(current);
}

void TreeView::deleteItem() {
  QModelIndex current = currentIndex();
  if (!current.isValid() || !m_treeModel) return;
  TreeItem* item = m_treeModel->itemFromIndex(current);
  if (item->readOnly) return;
  QString message = QString("Are you sure you want to delete '%1'?").arg(item->displayName());
  if (item->childCount() > 0) {
    message += QString("\nThis will also delete %1 child item(s).").arg(item->childCount());
  }
  QMessageBox::StandardButton reply =
    QMessageBox::question(this, "Delete Item", message, QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    m_treeModel->removeItem(current);
  }
}

void TreeView::expandAll() {
  QTreeView::expandAll();
}

void TreeView::collapseAll() {
  QTreeView::collapseAll();
}

void TreeView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_contextMenuEnabled && m_contextMenu) {
    m_contextMenu->exec(event->globalPos());
  }
}

void TreeView::mouseDoubleClickEvent(QMouseEvent* event) {
  QModelIndex index = indexAt(event->pos());
  if (index.isValid() && m_treeModel) {
    TreeItem* item = m_treeModel->itemFromIndex(index);
    if (item->readOnly) return;
    emit itemDoubleClicked(item);
  }
  QTreeView::mouseDoubleClickEvent(event);
}

void TreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous) {
  QTreeView::currentChanged(current, previous);
  if (m_treeModel) {
    TreeItem* currentItem = m_treeModel->itemFromIndex(current);
    TreeItem* previousItem = m_treeModel->itemFromIndex(previous);
    emit itemSelectionChanged(currentItem, previousItem);
  }
}

bool TreeView::selectItemByName(const QString& name, bool recursive) {
  if (!m_treeModel) return false;
  QModelIndex index = m_treeModel->findIndexByName(name, recursive);
  if (index.isValid()) {
    setCurrentIndex(index);
    selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    scrollTo(index, QAbstractItemView::EnsureVisible);
    return true;
  }
  return false;
}

void TreeView::setupContextMenu() {
  m_contextMenu = new QMenu(this);

  m_addFolderAction = m_contextMenu->addAction("Add Folder");
  //m_addItemAction = m_contextMenu->addAction("Add Item");
  m_contextMenu->addSeparator();
  m_renameAction = m_contextMenu->addAction("Rename");
  m_deleteAction = m_contextMenu->addAction("Delete");
  m_contextMenu->addSeparator();
  m_expandAllAction = m_contextMenu->addAction("Expand All");
  m_collapseAllAction = m_contextMenu->addAction("Collapse All");

  connect(m_addFolderAction, &QAction::triggered, this, &TreeView::addFolder_);
  //connect(m_addItemAction, &QAction::triggered, this, &TreeView::addItem_);
  connect(m_renameAction, &QAction::triggered, this, &TreeView::renameItem);
  connect(m_deleteAction, &QAction::triggered, this, &TreeView::deleteItem);
  connect(m_expandAllAction, &QAction::triggered, this, &TreeView::expandAll);
  connect(m_collapseAllAction, &QAction::triggered, this, &TreeView::collapseAll);

}

///////////////////////////////////////////////////////////////////////////////
