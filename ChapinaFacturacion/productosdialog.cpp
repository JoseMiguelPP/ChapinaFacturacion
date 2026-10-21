#include "productosdialog.h"
#include "database.h"
#include "modelosql.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

ProductosDialog::ProductosDialog(QWidget *parent)
    : QDialog(parent), editando(false)
{
    setWindowTitle("Productos e inventario");
    resize(900, 650);

    //      Tabla
    modelo = new ModeloSql(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla->setSelectionMode(QAbstractItemView::SingleSelection);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);
    tabla->horizontalHeader()->setStretchLastSection(true);

    //      Formulario
    txtCodigo = new QLineEdit;
    txtCodigo->setMaxLength(10);
    txtNombre = new QLineEdit;
    txtNombre->setMaxLength(100);

    spnCosto = new QDoubleSpinBox;
    spnCosto->setRange(0, 999999.99);
    spnCosto->setDecimals(2);
    spnCosto->setPrefix("Q ");

    spnPrecio = new QDoubleSpinBox;
    spnPrecio->setRange(0, 999999.99);
    spnPrecio->setDecimals(2);
    spnPrecio->setPrefix("Q ");

    spnExistencia = new QSpinBox;
    spnExistencia->setRange(0, 1000000);

    QFormLayout *formulario = new QFormLayout;
    formulario->addRow("Código:", txtCodigo);
    formulario->addRow("Nombre:", txtNombre);
    formulario->addRow("Costo:", spnCosto);
    formulario->addRow("Precio de venta:", spnPrecio);
    formulario->addRow("Existencia (unidades):", spnExistencia);

    QGroupBox *grupo = new QGroupBox("Datos del producto");
    grupo->setLayout(formulario);

    //      Botones
    QPushButton *btnNuevo = new QPushButton("Nuevo");
    btnGuardar = new QPushButton("Guardar");
    btnEliminar = new QPushButton("Eliminar");

    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addStretch();
    filaBotones->addWidget(btnNuevo);
    filaBotones->addWidget(btnGuardar);
    filaBotones->addWidget(btnEliminar);

    //      Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(tabla, 1);
    principal->addWidget(grupo);
    principal->addLayout(filaBotones);

    //      Conectar acciones
    connect(btnNuevo,    &QPushButton::clicked, this, &ProductosDialog::nuevo);
    connect(btnGuardar,  &QPushButton::clicked, this, &ProductosDialog::guardar);
    connect(btnEliminar, &QPushButton::clicked, this, &ProductosDialog::eliminar);
    connect(tabla, &QTableView::clicked, this, &ProductosDialog::filaSeleccionada);

    cargarTabla();
    limpiarFormulario();
}

//  CONSULTAR
void ProductosDialog::cargarTabla()
{
    modelo->setQuery("SELECT codigo, nombre, costo, precio, existencia "
                     "FROM producto ORDER BY nombre");

    modelo->setHeaderData(0, Qt::Horizontal, "Código");
    modelo->setHeaderData(1, Qt::Horizontal, "Nombre");
    modelo->setHeaderData(2, Qt::Horizontal, "Costo (Q)");
    modelo->setHeaderData(3, Qt::Horizontal, "Precio (Q)");
    modelo->setHeaderData(4, Qt::Horizontal, "Existencia");

    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(true);
}

//  Clic en una fila: pasa los datos al formulario
void ProductosDialog::filaSeleccionada(const QModelIndex &indice)
{
    int fila = indice.row();

    txtCodigo->setText(modelo->data(modelo->index(fila, 0)).toString());
    txtNombre->setText(modelo->data(modelo->index(fila, 1)).toString());
    spnCosto->setValue(modelo->data(modelo->index(fila, 2)).toDouble());
    spnPrecio->setValue(modelo->data(modelo->index(fila, 3)).toDouble());
    spnExistencia->setValue(modelo->data(modelo->index(fila, 4)).toInt());

    editando = true;
    txtCodigo->setReadOnly(true);
    btnGuardar->setText("Guardar cambios");
    btnEliminar->setEnabled(true);
}

void ProductosDialog::limpiarFormulario()
{
    txtCodigo->clear();
    txtNombre->clear();
    spnCosto->setValue(0);
    spnPrecio->setValue(0);
    spnExistencia->setValue(0);

    editando = false;
    txtCodigo->setReadOnly(false);
    btnGuardar->setText("Guardar");
    btnEliminar->setEnabled(false);
    tabla->clearSelection();
    txtCodigo->setFocus();
}

void ProductosDialog::nuevo()
{
    limpiarFormulario();
}

//  Validaciones
bool ProductosDialog::validarDatos()
{
    if (txtCodigo->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el código del producto.");
        txtCodigo->setFocus();
        return false;
    }
    if (txtNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Escribe el nombre del producto.");
        txtNombre->setFocus();
        return false;
    }
    if (spnPrecio->value() <= 0) {
        QMessageBox::warning(this, "Datos incorrectos", "El precio de venta debe ser mayor que cero.");
        spnPrecio->setFocus();
        return false;
    }
    return true;
}

//  REGISTRAR / MODIFICAR
void ProductosDialog::guardar()
{
    if (!validarDatos())
        return;

    QString codigo     = sqlTexto(txtCodigo->text().trimmed());
    QString nombre     = sqlTexto(txtNombre->text().trimmed());
    QString costo      = QString::number(spnCosto->value(), 'f', 2);
    QString precio     = QString::number(spnPrecio->value(), 'f', 2);
    QString existencia = QString::number(spnExistencia->value());

    QString sql;
    if (editando) {
        sql = "UPDATE producto SET nombre = " + nombre + ", costo = " + costo +
              ", precio = " + precio + ", existencia = " + existencia +
              " WHERE codigo = " + codigo;
    } else {
        sql = "INSERT INTO producto (codigo, nombre, costo, precio, existencia) VALUES (" +
              codigo + ", " + nombre + ", " + costo + ", " + precio + ", " + existencia + ")";
    }

    QSqlQuery consulta;
    if (!consulta.exec(sql)) {
        mostrarErrorBD(this, consulta.lastError(), "Ya existe un producto con ese código.");
        return;
    }

    QMessageBox::information(this, "Listo", editando ? "Producto actualizado correctamente."
                                                     : "Producto registrado correctamente.");
    cargarTabla();
    limpiarFormulario();
}

//  ELIMINAR
void ProductosDialog::eliminar()
{
    QString codigo = txtCodigo->text();

    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this, "Confirmar eliminación",
        "¿Seguro que deseas eliminar el producto " + txtNombre->text() + " (" + codigo + ")?");
    if (respuesta != QMessageBox::Yes)
        return;

    QSqlQuery consulta;
    if (!consulta.exec("DELETE FROM producto WHERE codigo = " + sqlTexto(codigo))) {
        mostrarErrorBD(this, consulta.lastError(), "",
                       "Este producto ya fue vendido en alguna factura, por lo que no se puede eliminar.");
        return;
    }

    cargarTabla();
    limpiarFormulario();
}
