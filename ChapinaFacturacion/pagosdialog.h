#ifndef PAGOSDIALOG_H
#define PAGOSDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableView;
class ModeloSql;

// Modulo Pagos: registrar pagos de las facturas y consultar saldos e historial.
class PagosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PagosDialog(QWidget *parent = nullptr);

private slots:
    void filaSeleccionada(const QModelIndex &indice);
    void registrarPago();

private:
    void cargarFacturas();
    void cargarPagos();           // historial de pagos de la factura seleccionada
    void limpiarSeleccion();

    QTableView *tabla;            // facturas
    ModeloSql  *modelo;
    QTableView *tablaPagos;       // pagos de la factura seleccionada
    ModeloSql  *modeloPagos;
    QLabel     *lblInfo;
    QDoubleSpinBox *spnMonto;
    QPushButton *btnRegistrar;

    int     idFactura;            // factura seleccionada (0 = ninguna)
    QString numeroFactura;
    QString condicion;            // CONTADO o CREDITO
    double  saldo;                // lo que falta por pagar
};

#endif // PAGOSDIALOG_H
