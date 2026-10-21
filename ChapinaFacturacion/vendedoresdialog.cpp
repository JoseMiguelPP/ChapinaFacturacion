#include "vendedoresdialog.h"
#include "database.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>

VendedoresDialog::VendedoresDialog(QWidget *parent)
    : QDialog(parent), editando(false)
{
    setWindowTitle("Vendedores");
    resize(900, 650);

    //Tabla
    modelo = new QSqlQueryModel(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla->setSelectionMode(QAbstractItemView::SingleSelection);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);
    tabla->horizontalHeader()->setStretchLastSection(true);

    //     Formulario
    txtCodigo    = new QLineEdit;
    txtDpi       = new QLineEdit;
    txtNombre    = new QLineEdit;
    txtTelefono  = new QLineEdit;
    txtDireccion = new QLineEdit;
    txtEmail     = new QLineEdit;

    txtCodigo->setMaxLength(10);
    txtDpi->setMaxLength(13);
    txtDpi->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,13}"), this));
    txtNombre->setMaxLength(100);
    txtTelefono->setMaxLength(20);
    txtDireccion->setMaxLength(150);
    txtEmail->setMaxLength(100);

    QFormLayout *formulario = new QFormLayout;
    formulario->addRow("Código de empleado:", txtCodigo);
    formulario->addRow("DPI (13 dígitos):", txtDpi);
    formulario->addRow("Nombre:", txtNombre);
    formulario->addRow("Teléfono:", txtTelefono);
    formulario->addRow("Dirección:", txtDireccion);
    formulario->addRow("E-mail:", txtEmail);

    QGroupBox *grupo = new QGroupBox("Datos del vendedor");
    grupo->setLayout(formulario);

    //      Botones
    QPushButton *btnNuevo = new QPushButton("Nuevo");
    btnGuardar = new QPushButton("Guardar");
    btnEliminar = new QPushButton("Eliminar");
    btnEliminar->setEnabled(false);

    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addStretch();
    filaBotones->addWidget(btnNuevo);
    filaBotones->addWidget(btnGuardar);
    filaBotones->addWidget(btnEliminar);

    //      Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(tabla, 1);        // el 1 hace que la tabla crezca con la ventana
    principal->addWidget(grupo);
    principal->addLayout(filaBotones);

    //      Conectar acciones
    connect(btnNuevo,    &QPushButton::clicked, this, &VendedoresDialog::nuevo);
    connect(btnGuardar,  &QPushButton::clicked, this, &VendedoresDialog::guardar);
    connect(btnEliminar, &QPushButton::clicked, this, &VendedoresDialog::eliminar);
    connect(tabla, &QTableView::clicked, this, &VendedoresDialog::filaSeleccionada);

    cargarTabla();
    limpiarFormulario();
}

//  CONSULTAR: llena la tabla

void VendedoresDialog::cargarTabla()
{
    modelo->setQuery("SELECT codigo_empleado, dpi, nombre, telefono, direccion, email "
                     "FROM vendedor ORDER BY nombre");

    modelo->setHeaderData(0, Qt::Horizontal, "Código");
    modelo->setHeaderData(1, Qt::Horizontal, "DPI");
    modelo->setHeaderData(2, Qt::Horizontal, "Nombre");
    modelo->setHeaderData(3, Qt::Horizontal, "Teléfono");
    modelo->setHeaderData(4, Qt::Horizontal, "Dirección");
    modelo->setHeaderData(5, Qt::Horizontal, "E-mail");

    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(true);
}

//  Clic en una fila: pasa los datos al formulario para modificar
void VendedoresDialog::filaSeleccionada(const QModelIndex &indice)
{
    int fila = indice.row();

    txtCodigo->setText(modelo->data(modelo->index(fila, 0)).toString());
    txtDpi->setText(modelo->data(modelo->index(fila, 1)).toString());
    txtNombre->setText(modelo->data(modelo->index(fila, 2)).toString());
    txtTelefono->setText(modelo->data(modelo->index(fila, 3)).toString());
    txtDireccion->setText(modelo->data(modelo->index(fila, 4)).toString());
    txtEmail->setText(modelo->data(modelo->index(fila, 5)).toString());

    editando = true;
    txtCodigo->setReadOnly(true);     // el codigo es la llave: no se cambia
    btnGuardar->setText("Guardar cambios");
    btnEliminar->setEnabled(true);
}

//  Deja el formulario vacio, listo para registrar un vendedor nuevo
void VendedoresDialog::limpiarFormulario()
{
    txtCodigo->clear();
    txtDpi->clear();
    txtNombre->clear();
    txtTelefono->clear();
    txtDireccion->clear();
    txtEmail->clear();

    editando = false;
    txtCodigo->setReadOnly(false);
    btnGuardar->setText("Guardar");
    btnEliminar->setEnabled(false);
    tabla->clearSelection();
    txtCodigo->setFocus();
}

void VendedoresDialog::nuevo()
{
    limpiarFormulario();
}

//  Validaciones antes de guardar
bool VendedoresDialog::validarDatos()
{
    if (txtCodigo->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el código de empleado.");
        txtCodigo->setFocus();
        return false;
    }
    if (txtDpi->text().length() != 13) {
        QMessageBox::warning(this, "Datos incorrectos", "El DPI debe tener exactamente 13 dígitos.");
        txtDpi->setFocus();
        return false;
    }
    if (txtNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el nombre del vendedor.");
        txtNombre->setFocus();
        return false;
    }
    return true;
}

//  REGISTRAR / MODIFICAR
void VendedoresDialog::guardar()
{
    if (!validarDatos())
        return;

    QString codigo    = sqlTexto(txtCodigo->text().trimmed());
    QString dpi       = sqlTexto(txtDpi->text());
    QString nombre    = sqlTexto(txtNombre->text().trimmed());
    QString telefono  = sqlTexto(txtTelefono->text().trimmed());
    QString direccion = sqlTexto(txtDireccion->text().trimmed());
    QString email     = sqlTexto(txtEmail->text().trimmed());

    QString sql;
    if (editando) {
        sql = "UPDATE vendedor SET dpi = " + dpi + ", nombre = " + nombre +
              ", telefono = " + telefono + ", direccion = " + direccion +
              ", email = " + email + " WHERE codigo_empleado = " + codigo;
    } else {
        sql = "INSERT INTO vendedor (codigo_empleado, dpi, nombre, telefono, direccion, email) "
              "VALUES (" + codigo + ", " + dpi + ", " + nombre + ", " + telefono + ", " +
              direccion + ", " + email + ")";
    }

    QSqlQuery consulta;
    if (!consulta.exec(sql)) {
        mostrarError(consulta);
        return;
    }

    QMessageBox::information(this, "Listo", editando ? "Vendedor actualizado correctamente."
                                                     : "Vendedor registrado correctamente.");
    cargarTabla();
    limpiarFormulario();
}

//  ELIMINAR
void VendedoresDialog::eliminar()
{
    QString codigo = txtCodigo->text();
    QString nombre = txtNombre->text();

    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this, "Confirmar eliminación",
        "¿Seguro que deseas eliminar al vendedor " + nombre + " (" + codigo + ")?\n\n"
        "Sus clientes quedarán sin vendedor asignado.");

    if (respuesta != QMessageBox::Yes)
        return;

    QSqlQuery consulta;
    if (!consulta.exec("DELETE FROM vendedor WHERE codigo_empleado = " + sqlTexto(codigo))) {
        mostrarError(consulta);
        return;
    }

    cargarTabla();
    limpiarFormulario();
}

//  Traduce los errores de MySQL a mensajes que entienda el usuario
void VendedoresDialog::mostrarError(const QSqlQuery &consulta)
{
    QString codigoError = consulta.lastError().nativeErrorCode();
    QString mensaje;

    if (codigoError == "1062") {
        mensaje = "Ya existe un vendedor con ese código o con ese DPI.";
    } else if (codigoError == "1451") {
        mensaje = "Este vendedor tiene facturas registradas, por lo que no se puede eliminar.";
    } else {
        mensaje = mensajeDeError(consulta.lastError());
    }

    QMessageBox::critical(this, "Error", mensaje);
}
