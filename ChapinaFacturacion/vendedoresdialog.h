#ifndef VENDEDORESDIALOG_H
#define VENDEDORESDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QLineEdit;
class QPushButton;
class QSqlQuery;
class QSqlQueryModel;
class QTableView;

// Ventana del modulo Vendedores: registrar, consultar, modificar y eliminar.
// Este mismo patron se repetira en Clientes y Productos.
class VendedoresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VendedoresDialog(QWidget *parent = nullptr);

private slots:
    void nuevo();                                  // boton "Nuevo"
    void guardar();                                // boton "Guardar"
    void eliminar();                               // boton "Eliminar"
    void filaSeleccionada(const QModelIndex &indice); // clic en la tabla

private:
    void cargarTabla();          // llena la tabla con los vendedores (aplica la busqueda)
    void limpiarFormulario();    // deja el formulario listo para un vendedor nuevo
    bool validarDatos();         // revisa que los datos esten completos y correctos
    void mostrarError(const QSqlQuery &consulta);   // mensaje amigable de error de la BD

    // Elementos visuales
    QTableView     *tabla;
    QSqlQueryModel *modelo;
    QLineEdit *txtCodigo;
    QLineEdit *txtDpi;
    QLineEdit *txtNombre;
    QLineEdit *txtTelefono;
    QLineEdit *txtDireccion;
    QLineEdit *txtEmail;
    QPushButton *btnGuardar;
    QPushButton *btnEliminar;

    bool editando;   // false = registrando uno nuevo | true = modificando uno existente
};

#endif // VENDEDORESDIALOG_H
