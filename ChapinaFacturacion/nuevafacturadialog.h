#ifndef NUEVAFACTURADIALOG_H
#define NUEVAFACTURADIALOG_H

#include <QDialog>
#include <QString>
#include <QVector>

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;

// Una linea del detalle de la factura (un producto y su cantidad)
struct LineaFactura
{
    QString codigo;
    QString nombre;
    int     cantidad;
    double  precio;
};

// Ventana para generar una factura nueva (al contado o a credito)
class NuevaFacturaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NuevaFacturaDialog(QWidget *parent = nullptr);

private slots:
    void clienteCambiado();     // al elegir un cliente se sugiere su vendedor
    void condicionCambiada();   // contado o credito (el plazo solo aplica a credito)
    void agregarProducto();
    void quitarLinea();
    void guardar();

private:
    void cargarCombos();
    void mostrarLineas();       // dibuja la tabla de productos y los totales

    QComboBox *cmbCliente;
    QComboBox *cmbVendedor;
    QComboBox *cmbCondicion;
    QSpinBox  *spnPlazo;
    QComboBox *cmbProducto;
    QSpinBox  *spnCantidad;
    QTableWidget *tablaLineas;
    QLabel *lblSubtotal;
    QLabel *lblIva;
    QLabel *lblTotal;
    QPushButton *btnGuardar;

    QVector<LineaFactura> lineas;   // productos agregados a la factura
};

#endif // NUEVAFACTURADIALOG_H
