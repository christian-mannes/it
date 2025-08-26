#include "paramsmodel.h"
#include <QSize>
#include <QBrush>
#include <QFont>

// https://doc.qt.io/qt-6/modelview.html#3-1-treeview

void ParamsModel::setFunction(Function *f) {
  beginResetModel();
  function = f;
  endResetModel();
}
int ParamsModel::rowCount(const QModelIndex &parent) const {
  if (function == nullptr) return 0;
  return function->args.count();
}
int ParamsModel::columnCount(const QModelIndex &parent) const {
  if (function == nullptr) return 0;
  return 2;
}

// https://doc.qt.io/qt-6/modelview.html#3-1-treeview

QVariant ParamsModel::data(const QModelIndex &index, int role) const {
  if (function == nullptr) return QVariant();
  if (role == Qt::DisplayRole) {
    int row = index.row();
    int col = index.column();
    if (col == 0) {
      return QString(function->args.getArgAt(row)->name().c_str());
    } else {
      return QString(function->args.getArgAt(row)->toString().c_str());
    }
  } else if (role == Qt::SizeHintRole) {
    if (index.column() == 0) return QSize(120, 0);
    if (index.column() == 1) return QSize(150, 0);
  } else if (role == Qt::FontRole) {
    if (index.column() == 0) {
      QFont boldFont;
      boldFont.setBold(true);
      return boldFont;
    }
  } else if (role == Qt::ForegroundRole) {
    if (index.column() == 0) return  QBrush(QColor::fromRgb(100, 100, 100));
  }
  return QVariant();
}

bool ParamsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role == Qt::EditRole) {
    if (!checkIndex(index)) return false;
    //save value from editor to member m_gridData
    int row = index.row();
    if (row >= function->args.count()) return false;
    std::string s = value.toString().toStdString();
    try {
      function->args.getArgAt(row)->parse(s.c_str());
      function->other->args.getArgAt(row)->parse(s.c_str());
    } catch (...) {
      return false;
    }
    return true;
  }
  return false;
}

Qt::ItemFlags ParamsModel::flags(const QModelIndex &index) const {
  if (index.column() == 1)
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
  return QAbstractTableModel::flags(index);
}
