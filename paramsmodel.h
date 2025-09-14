#ifndef PARAMSMODEL_H
#define PARAMSMODEL_H

#include <QAbstractTableModel>
#include "Function.h"

class ParamsModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit ParamsModel(QObject *parent = nullptr) { function = nullptr; }
  void setFunction(Function *f);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
  Function *function;
};

#endif // PARAMSMODEL_H
