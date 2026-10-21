#include "clientesdialog.h"
#include "database.h"
#include "modelosql.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

ClientesDialog::ClientesDialog(QWidget *parent)
    : QDialog(parent), editando(false), idActual(0)
{
    setWindowTitle("Clientes");
    resize(1000, 720);

    modelo = new ModeloSql(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla->setSelectionMode(QAbstractItemView::SingleSelection);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);
    tabla->horizontalHeader()->setStretchLastSection(true);

    txtNombre    = new QLineEdit;  txtNombre->setMaxLength(100);
    txtNit       = new QLineEdit;  txtNit->setMaxLength(15);
    txtTelefono  = new QLineEdit;  txtTelefono->setMaxLength(20);
    txtDireccion = new QLineEdit;  txtDireccion->setMaxLength(150);
    txtContacto  = new QLineEdit;  txtContacto->setMaxLength(100);
    txtEmail     = new QLineEdit;  txtEmail->setMaxLength(100);
    txtWeb       = new QLineEdit;  txtWeb->setMaxLength(100);
    cmbVendedor  = new QComboBox;

    QFormLayout *formulario = new QFormLayout;
    formulario->addRow("Nombre:", txtNombre);
    formulario->addRow("NIT:", txtNit);
    formulario->addRow("Teléfono:", txtTelefono);
    formulario->addRow("Dirección:", txtDireccion);
    formulario->addRow("Persona de contacto:", txtContacto);
    formulario->addRow("E-mail:", txtEmail);
    formulario->addRow("Página web:", txtWeb);
    formulario->addRow("Vendedor asignado:", cmbVendedor);

    QGroupBox *grupo = new QGroupBox("Datos del cliente");
    grupo->setLayout(formulario);

    QPushButton *btnNuevo = new QPushButton("Nuevo");
    btnGuardar = new QPushButton("Guardar");
    btnEliminar = new QPushButton("Eliminar");

    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addStretch();
    filaBotones->addWidget(btnNuevo);
    filaBotones->addWidget(btnGuardar);
    filaBotones->addWidget(btnEliminar);

    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(tabla, 1);
    principal->addWidget(grupo);
    principal->addLayout(filaBotones);

    connect(btnNuevo,    &QPushButton::clicked, this, &ClientesDialog::nuevo);
    connect(btnGuardar,  &QPushButton::clicked, this, &ClientesDialog::guardar);
    connect(btnEliminar, &QPushButton::clicked, this, &ClientesDialog::eliminar);
    connect(tabla, &QTableView::clicked, this, &ClientesDialog::filaSeleccionada);

    cargarVendedores();
    cargarTabla();
    limpiarFormulario();
}

//  Llena la lista desplegable con los vendedores
void ClientesDialog::cargarVendedores()
{
    cmbVendedor->clear();
    cmbVendedor->addItem("(sin vendedor)", "");

    QSqlQuery consulta("SELECT codigo_empleado, nombre FROM vendedor ORDER BY nombre");
    while (consulta.next()) {
        QString codigo = consulta.value(0).toString();
        QString nombre = consulta.value(1).toString();
        cmbVendedor->addItem(codigo + " - " + nombre, codigo);
    }
}

//  CONSULTAR
void ClientesDialog::cargarTabla()
{
    modelo->setQuery("SELECT c.id_cliente, c.nombre, c.nit, c.telefono, c.direccion, "
                     "c.persona_contacto, c.email, c.pagina_web, "
                     "IFNULL(v.nombre, '(sin vendedor)'), IFNULL(c.codigo_vendedor, '') "
                     "FROM cliente c "
                     "LEFT JOIN vendedor v ON v.codigo_empleado = c.codigo_vendedor "
                     "ORDER BY c.nombre");

    modelo->setHeaderData(1, Qt::Horizontal, "Nombre");
    modelo->setHeaderData(2, Qt::Horizontal, "NIT");
    modelo->setHeaderData(3, Qt::Horizontal, "Teléfono");
    modelo->setHeaderData(4, Qt::Horizontal, "Dirección");
    modelo->setHeaderData(5, Qt::Horizontal, "Contacto");
    modelo->setHeaderData(6, Qt::Horizontal, "E-mail");
    modelo->setHeaderData(7, Qt::Horizontal, "Página web");
    modelo->setHeaderData(8, Qt::Horizontal, "Vendedor");

    tabla->setColumnHidden(0, true);
    tabla->setColumnHidden(9, true);
    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(false);
    tabla->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
}

//  Clic en una fila: pasa los datos al formulario
void ClientesDialog::filaSeleccionada(const QModelIndex &indice)
{
    int fila = indice.row();

    idActual = modelo->data(modelo->index(fila, 0)).toInt();
    txtNombre->setText(modelo->data(modelo->index(fila, 1)).toString());
    txtNit->setText(modelo->data(modelo->index(fila, 2)).toString());
    txtTelefono->setText(modelo->data(modelo->index(fila, 3)).toString());
    txtDireccion->setText(modelo->data(modelo->index(fila, 4)).toString());
    txtContacto->setText(modelo->data(modelo->index(fila, 5)).toString());
    txtEmail->setText(modelo->data(modelo->index(fila, 6)).toString());
    txtWeb->setText(modelo->data(modelo->index(fila, 7)).toString());

    QString codigoVendedor = modelo->data(modelo->index(fila, 9)).toString();
    int posicion = cmbVendedor->findData(codigoVendedor);
    cmbVendedor->setCurrentIndex(posicion >= 0 ? posicion : 0);

    editando = true;
    btnGuardar->setText("Guardar cambios");
    btnEliminar->setEnabled(true);
}

void ClientesDialog::limpiarFormulario()
{
    txtNombre->clear();
    txtNit->clear();
    txtTelefono->clear();
    txtDireccion->clear();
    txtContacto->clear();
    txtEmail->clear();
    txtWeb->clear();
    cmbVendedor->setCurrentIndex(0);

    editando = false;
    idActual = 0;
    btnGuardar->setText("Guardar");
    btnEliminar->setEnabled(false);
    tabla->clearSelection();
    txtNombre->setFocus();
}

void ClientesDialog::nuevo()
{
    limpiarFormulario();
}

bool ClientesDialog::validarDatos()
{
    if (txtNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el nombre del cliente.");
        txtNombre->setFocus();
        return false;
    }
    if (txtNit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el NIT del cliente.");
        txtNit->setFocus();
        return false;
    }
    return true;
}

//                                                 ---
//  REGISTRAR / MODIFICAR
//                                                 ---
void ClientesDialog::guardar()
{
    if (!validarDatos())
        return;

    QString nombre   = sqlTexto(txtNombre->text().trimmed());
    QString nit      = sqlTexto(txtNit->text().trimmed());
    QString telefono = sqlTexto(txtTelefono->text().trimmed());
    QString direccion = sqlTexto(txtDireccion->text().trimmed());
    QString contacto = sqlTexto(txtContacto->text().trimmed());
    QString email    = sqlTexto(txtEmail->text().trimmed());
    QString web      = sqlTexto(txtWeb->text().trimmed());

    // Si no se eligio vendedor se guarda NULL (vacio) en la base de datos
    QString codigoVendedor = cmbVendedor->currentData().toString();
    QString vendedor = codigoVendedor.isEmpty() ? "NULL" : sqlTexto(codigoVendedor);

    QString sql;
    if (editando) {
        sql = "UPDATE cliente SET nombre = " + nombre + ", nit = " + nit +
              ", telefono = " + telefono + ", direccion = " + direccion +
              ", persona_contacto = " + contacto + ", email = " + email +
              ", pagina_web = " + web + ", codigo_vendedor = " + vendedor +
              " WHERE id_cliente = " + QString::number(idActual);
    } else {
        sql = "INSERT INTO cliente (nombre, nit, telefono, direccion, persona_contacto, "
              "email, pagina_web, codigo_vendedor) VALUES (" +
              nombre + ", " + nit + ", " + telefono + ", " + direccion + ", " +
              contacto + ", " + email + ", " + web + ", " + vendedor + ")";
    }

    QSqlQuery consulta;
    if (!consulta.exec(sql)) {
        mostrarErrorBD(this, consulta.lastError(), "Ya existe un cliente con ese NIT.");
        return;
    }

    QMessageBox::information(this, "Listo", editando ? "Cliente actualizado correctamente."
                                                     : "Cliente registrado correctamente.");
    cargarTabla();
    limpiarFormulario();
}

//  ELIMINAR
void ClientesDialog::eliminar()
{
    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this, "Confirmar eliminación",
        "¿Seguro que deseas eliminar al cliente " + txtNombre->text() + "?");
    if (respuesta != QMessageBox::Yes)
        return;

    QSqlQuery consulta;
    if (!consulta.exec("DELETE FROM cliente WHERE id_cliente = " + QString::number(idActual))) {
        mostrarErrorBD(this, consulta.lastError(), "",
                       "Este cliente tiene facturas registradas, por lo que no se puede eliminar.");
        return;
    }

    cargarTabla();
    limpiarFormulario();
}
