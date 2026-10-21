#ifndef CLIENTESDIALOG_H
#define CLIENTESDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QComboBox;
class QLineEdit;
class QPushButton;
class QTableView;
class ModeloSql;

class ClientesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientesDialog(QWidget *parent = nullptr);

private slots:
    void nuevo();
    void guardar();
    void eliminar();
    void filaSeleccionada(const QModelIndex &indice);

private:
    void cargarVendedores();
    void cargarTabla();
    void limpiarFormulario();
    bool validarDatos();

    QTableView *tabla;
    ModeloSql  *modelo;
    QLineEdit *txtNombre;
    QLineEdit *txtNit;
    QLineEdit *txtTelefono;
    QLineEdit *txtDireccion;
    QLineEdit *txtContacto;
    QLineEdit *txtEmail;
    QLineEdit *txtWeb;
    QComboBox *cmbVendedor;
    QPushButton *btnGuardar;
    QPushButton *btnEliminar;

    bool editando;
    int  idActual;
};

#endif