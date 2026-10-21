#include "modelosql.h"

#include <QDate>
#include <QMetaType>

QVariant ModeloSql::data(const QModelIndex &indice, int rol) const
{
    if (rol == Qt::DisplayRole || rol == Qt::TextAlignmentRole) {
        QVariant valor = QSqlQueryModel::data(indice, Qt::DisplayRole);
        int tipo = valor.typeId();

        if (rol == Qt::TextAlignmentRole) {
            // Los numeros van pegados a la derecha
            if (tipo == QMetaType::Double || tipo == QMetaType::Int ||
                tipo == QMetaType::LongLong || tipo == QMetaType::UInt ||
                tipo == QMetaType::ULongLong)
                return int(Qt::AlignRight | Qt::AlignVCenter);
            return QSqlQueryModel::data(indice, rol);
        }

        if (tipo == QMetaType::Double)
            return QString::number(valor.toDouble(), 'f', 2);
        if (tipo == QMetaType::QDate)
            return valor.toDate().toString("dd/MM/yyyy");
        return valor;
    }
    return QSqlQueryModel::data(indice, rol);
}
