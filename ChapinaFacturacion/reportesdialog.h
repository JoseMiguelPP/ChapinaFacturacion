#ifndef REPORTESDIALOG_H
#define REPORTESDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QVector>

class QComboBox;
class QTableView;
class ModeloSql;

// Modulo Reportes: los 6 reportes que pide el enunciado.
class ReportesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportesDialog(QWidget *parent = nullptr);

private slots:
    void mostrarReporte();

private:
    // Descripcion de un reporte
    struct Reporte {
        QString titulo;           // nombre que se ve en la lista
        QString sql;              // consulta que obtiene los datos
        QStringList encabezados;  // titulos de las columnas
    };

    QVector<Reporte> reportes;

    QComboBox  *cmbReporte;
    QTableView *tabla;
    ModeloSql  *modelo;
};

#endif // REPORTESDIALOG_H
