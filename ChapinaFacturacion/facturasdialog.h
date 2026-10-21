#ifndef FACTURASDIALOG_H
#define FACTURASDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QLabel;
class QPushButton;
class QTableView;
class ModeloSql;

// Modulo Facturacion: lista de facturas con su detalle.
// Desde aqui se genera una factura nueva o se anula una existente.
class FacturasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FacturasDialog(QWidget *parent = nullptr);

private slots:
    void nuevaFactura();
    void anularFactura();
    void filaSeleccionada(const QModelIndex &indice);

private:
    void cargarTabla();
    void cargarDetalle();

    QTableView *tabla;
    ModeloSql  *modelo;
    QTableView *tablaDetalle;
    ModeloSql  *modeloDetalle;
    QLabel     *lblResumen;
    QPushButton *btnAnular;

    int     idSeleccionada;   // id de la factura seleccionada (0 = ninguna)
    QString numeroSeleccionado;
    QString estadoSeleccionado;
};

#endif // FACTURASDIALOG_H
