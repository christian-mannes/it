#pragma once

#include <QTreeView>
#include <QAbstractItemModel>
#include <QMimeData>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QApplication>
#include <functional>

///////////////////////////////////////////////////////////////////////////////

class TreeItem {
public:
  enum ItemType {
    Folder,
    Item
  };

  TreeItem(const QString& displayName, ItemType type = Item, void* data = nullptr);
  ~TreeItem();

  // Basic properties
  QString displayName() const { return m_displayName; }
  void setDisplayName(const QString& name) { m_displayName = name; }

  ItemType type() const { return m_type; }
  bool isFolder() const { return m_type == Folder; }
  bool isItem() const { return m_type == Item; }

  void* data() const { return m_data; }
  void setData(void* data) { m_data = data; }

  // Hierarchy management
  TreeItem* parent() const { return m_parent; }
  const QList<TreeItem*>& children() const { return m_children; }
  int childCount() const { return m_children.size(); }
  TreeItem* child(int index) const;
  int indexInParent() const;

  // Manipulation methods
  void appendChild(TreeItem* child);
  void insertChild(int index, TreeItem* child);
  void removeChild(TreeItem* child);
  void removeChild(int index);

  // Move operations
  void moveTo(TreeItem* newParent, int newIndex = -1);

  // Traversal helpers
  TreeItem* findChild(const QString& displayName, bool recursive = false) const;
  QList<TreeItem*> findAll(std::function<bool(TreeItem*)> predicate, bool recursive = true) const;

  // Path operations
  QString path(const QString& separator = "/") const;
  TreeItem* itemAtPath(const QString& path, const QString& separator = "/") const;

public:
  bool readOnly;
private:
  QString m_displayName;
  ItemType m_type;
  void* m_data;
  TreeItem* m_parent;
  QList<TreeItem*> m_children;
};

///////////////////////////////////////////////////////////////////////////////

class TreeModel : public QAbstractItemModel {
  Q_OBJECT

public:
  explicit TreeModel(QObject* parent = nullptr);
  ~TreeModel();

  // QAbstractItemModel interface
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Drag and drop support
  Qt::DropActions supportedDropActions() const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

  // Custom methods
  void setRootItem(TreeItem* root);
  TreeItem* rootItem() const { return m_rootItem; }
  TreeItem* itemFromIndex(const QModelIndex& index) const;
  QModelIndex indexFromItem(TreeItem* item) const;

  // Manipulation methods
  QModelIndex addFolder(const QModelIndex& parent = QModelIndex(), const QString& name = "New Folder");
  QModelIndex addItem(const QModelIndex& parent = QModelIndex(), const QString& name = "New Item", void* data = nullptr);
  void removeItem(const QModelIndex& index);

  TreeItem* findItemByName(const QString& name, bool recursive = true) const;
  QModelIndex findIndexByName(const QString& name, bool recursive = true) const;
  bool selectItemByName(const QString& name, bool recursive = true);

signals:
  void itemRenamed(TreeItem* item, const QString& oldName, const QString& newName);
  void itemMoved(TreeItem* item, TreeItem* oldParent, TreeItem* newParent);
  void itemAdded(TreeItem* item, TreeItem* parent);
  void itemRemoved(TreeItem* item, TreeItem* parent);

private:
  TreeItem* m_rootItem;
  static const QString MIME_TYPE;
};

///////////////////////////////////////////////////////////////////////////////

class TreeView : public QTreeView {
  Q_OBJECT

public:
  explicit TreeView(QWidget* parent = nullptr);

  // Setup
  void setTreeModel(TreeModel* model);
  TreeModel* treeModel() const;
  QSize sizeHint() const override { return QSize(preferredWidth, preferredHeight); }
  int preferredWidth;
  int preferredHeight;
  void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
  void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  // Access to current selection
  TreeItem* currentItem() const;
  QList<TreeItem*> selectedItems() const;

  // Configuration
  void setContextMenuEnabled(bool enabled) { m_contextMenuEnabled = enabled; }
  void setDragDropEnabled(bool enabled);
  bool selectItemByName(const QString& name, bool recursive = true);
  void addItem(const QString &name);
  void addFolder(const QString &name);

public slots:
  void addFolder_();
  void addItem_();
  void renameItem();
  void deleteItem();
  void expandAll();
  void collapseAll();

signals:
  void itemDoubleClicked(TreeItem* item);
  void itemSelectionChanged(TreeItem* current, TreeItem* previous);

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

private:
  void setupContextMenu();

  TreeModel* m_treeModel;
  QMenu* m_contextMenu;
  QAction* m_addFolderAction;
  QAction* m_addItemAction;
  QAction* m_renameAction;
  QAction* m_deleteAction;
  QAction* m_expandAllAction;
  QAction* m_collapseAllAction;

  bool m_contextMenuEnabled;
};

///////////////////////////////////////////////////////////////////////////////
