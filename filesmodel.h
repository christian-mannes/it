#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QAbstractTableModel>

struct FileEntry {
  QString name;
  QString file;
  bool selected;
  FileEntry(QString n, QString f, bool s) {
    name = n; file = f; selected = s;
  }
};

class FilesModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit FilesModel(QObject *parent = nullptr) {}
  void read();
  void write();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
  QString filepath;
  QList<FileEntry> files;
};

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QVector>
#include <QTreeView>

class TreeItem {
public:
  explicit TreeItem(const QStringList& data, TreeItem* parent = nullptr);
  ~TreeItem();

  void appendChild(TreeItem* child);
  void removeChild(int row);
  TreeItem* child(int row) const;
  int childCount() const;

  QVariant data(int column) const;
  bool setData(int column, const QVariant& value);
  int columnCount() const;

  TreeItem* parentItem() const;
  int row() const;
private:
  QVector<TreeItem*> childItems;
  QStringList itemData;
  TreeItem* _parentItem;
};

class TreeModel : public QAbstractItemModel {
  Q_OBJECT
public:
  explicit TreeModel(const QStringList& headers, QObject* parent = nullptr);
  ~TreeModel();
  // Overrides
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  // Optional: Editing support
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  // Custom functions for data manipulation
  bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  // Helper functions
  TreeItem* getItem(const QModelIndex& index) const;
  QModelIndex addChild(const QModelIndex& parent, const QStringList& data);
  void setupModelData(const QStringList& lines, TreeItem* parent);

public:
  void addFolder(const QString &name);
  void addEntry(const QString &name);
  bool selectItem(QTreeView *treeView, const QString &value, int col);

private:
  TreeItem* rootItem;
  QStringList headers;
  QModelIndex _folder;
  QModelIndex _root;
};

#endif // FILESMODEL_H
