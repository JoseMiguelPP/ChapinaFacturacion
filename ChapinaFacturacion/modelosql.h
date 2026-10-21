#ifndef MODELOSQL_H
#define MODELOSQL_H

#include <QSqlQueryModel>

// Igual que QSqlQueryModel, pero muestra los numeros decimales con 2 decimales,
// las fechas como dia/mes/año y alinea los numeros a la derecha.
// Se usa en todas las tablas del sistema.
class ModeloSql : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit ModeloSql(QObject *parent = nullptr) : QSqlQueryModel(parent) {}

    QVariant data(const QModelIndex &indice, int rol = Qt::DisplayRole) const override;
};

#endif // MODELOSQL_H
