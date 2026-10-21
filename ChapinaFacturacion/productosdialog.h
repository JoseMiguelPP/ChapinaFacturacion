#ifndef PRODUCTOSDIALOG_H
#define PRODUCTOSDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableView;
class ModeloSql;

// Modulo Productos / Inventario: registrar, consultar, modificar y eliminar.
class ProductosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProductosDialog(QWidget *parent = nullptr);

private slots:
    void nuevo();
    void guardar();
    void eliminar();
    void filaSeleccionada(const QModelIndex &indice);

private:
    void cargarTabla();
    void limpiarFormulario();
    bool validarDatos();

    QTableView *tabla;
    ModeloSql  *modelo;
    QLineEdit *txtCodigo;
    QLineEdit *txtNombre;
    QDoubleSpinBox *spnCosto;
    QDoubleSpinBox *spnPrecio;
    QSpinBox *spnExistencia;
    QPushButton *btnGuardar;
    QPushButton *btnEliminar;

    bool editando;   // false = producto nuevo | true = modificando uno existente
};

#endif // PRODUCTOSDIALOG_H
