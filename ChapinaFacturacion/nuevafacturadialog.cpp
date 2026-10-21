#include "nuevafacturadialog.h"
#include "database.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTableWidget>
#include <QVBoxLayout>
#include <cmath>

NuevaFacturaDialog::NuevaFacturaDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Nueva factura");
    resize(900, 720);

    //      Datos generales
    cmbCliente   = new QComboBox;
    cmbVendedor  = new QComboBox;
    cmbCondicion = new QComboBox;
    cmbCondicion->addItem("Contado", "CONTADO");
    cmbCondicion->addItem("Crédito", "CREDITO");

    spnPlazo = new QSpinBox;
    spnPlazo->setRange(1, 365);
    spnPlazo->setValue(30);
    spnPlazo->setSuffix(" días");
    spnPlazo->setEnabled(false);          // solo se usa en credito

    QFormLayout *datosGenerales = new QFormLayout;
    datosGenerales->addRow("Cliente:", cmbCliente);
    datosGenerales->addRow("Vendedor:", cmbVendedor);
    datosGenerales->addRow("Condición de pago:", cmbCondicion);
    datosGenerales->addRow("Plazo (crédito):", spnPlazo);

    QGroupBox *grupoGeneral = new QGroupBox("Datos de la factura");
    grupoGeneral->setLayout(datosGenerales);

    //      Agregar productos
    cmbProducto = new QComboBox;
    cmbProducto->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    spnCantidad = new QSpinBox;
    spnCantidad->setRange(1, 100000);
    QPushButton *btnAgregar = new QPushButton("Agregar");

    QHBoxLayout *filaProducto = new QHBoxLayout;
    filaProducto->addWidget(new QLabel("Producto:"));
    filaProducto->addWidget(cmbProducto, 1);
    filaProducto->addWidget(new QLabel("Cantidad:"));
    filaProducto->addWidget(spnCantidad);
    filaProducto->addWidget(btnAgregar);

    //      Tabla del detalle
    tablaLineas = new QTableWidget(0, 5);
    tablaLineas->setHorizontalHeaderLabels({"Código", "Producto", "Cantidad", "Precio", "Subtotal"});
    tablaLineas->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaLineas->setSelectionMode(QAbstractItemView::SingleSelection);
    tablaLineas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaLineas->setAlternatingRowColors(true);
    tablaLineas->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    QPushButton *btnQuitar = new QPushButton("Quitar producto seleccionado");

    //      Totales
    lblSubtotal = new QLabel;
    lblIva      = new QLabel;
    lblTotal    = new QLabel;
    lblTotal->setStyleSheet("font-size: 18px; font-weight: bold; color: #1b5e20;");

    QFormLayout *totales = new QFormLayout;
    totales->addRow("Subtotal:", lblSubtotal);
    totales->addRow("IVA (12%):", lblIva);
    totales->addRow("TOTAL:", lblTotal);

    //      Botones
    btnGuardar = new QPushButton("Guardar factura");
    QPushButton *btnCancelar = new QPushButton("Cancelar");

    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addWidget(btnQuitar);
    filaBotones->addStretch();
    filaBotones->addLayout(totales);
    filaBotones->addSpacing(30);
    filaBotones->addWidget(btnCancelar);
    filaBotones->addWidget(btnGuardar);

    //      Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(grupoGeneral);
    principal->addLayout(filaProducto);
    principal->addWidget(tablaLineas, 1);
    principal->addLayout(filaBotones);

    //      Conectar acciones
    connect(cmbCliente,   &QComboBox::currentIndexChanged, this, &NuevaFacturaDialog::clienteCambiado);
    connect(cmbCondicion, &QComboBox::currentIndexChanged, this, &NuevaFacturaDialog::condicionCambiada);
    connect(btnAgregar,   &QPushButton::clicked, this, &NuevaFacturaDialog::agregarProducto);
    connect(btnQuitar,    &QPushButton::clicked, this, &NuevaFacturaDialog::quitarLinea);
    connect(btnGuardar,   &QPushButton::clicked, this, &NuevaFacturaDialog::guardar);
    connect(btnCancelar,  &QPushButton::clicked, this, &QDialog::reject);

    cargarCombos();
    mostrarLineas();
}

//  Llena las listas desplegables con datos de la base
void NuevaFacturaDialog::cargarCombos()
{
    QSqlQuery consulta;

    // Clientes: se ve "Nombre (NIT ...)", se guarda el id
    consulta.exec("SELECT id_cliente, nombre, nit FROM cliente ORDER BY nombre");
    while (consulta.next()) {
        cmbCliente->addItem(consulta.value(1).toString() + "  (NIT " + consulta.value(2).toString() + ")",
                            consulta.value(0).toInt());
    }

    // Vendedores: se ve "V001 - Nombre", se guarda el codigo
    consulta.exec("SELECT codigo_empleado, nombre FROM vendedor ORDER BY nombre");
    while (consulta.next()) {
        QString codigo = consulta.value(0).toString();
        cmbVendedor->addItem(codigo + " - " + consulta.value(1).toString(), codigo);
    }

    // Productos: solo los que tienen existencia
    consulta.exec("SELECT codigo, nombre, precio, existencia FROM producto "
                  "WHERE existencia > 0 ORDER BY nombre");
    while (consulta.next()) {
        QString texto = consulta.value(1).toString() + "   " +
                        formatoQ(consulta.value(2).toDouble()) +
                        "   (existencia: " + consulta.value(3).toString() + ")";
        cmbProducto->addItem(texto, consulta.value(0).toString());
    }

    clienteCambiado();   // selecciona el vendedor del primer cliente
}

// Al elegir un cliente se selecciona automaticamente su vendedor asignado
void NuevaFacturaDialog::clienteCambiado()
{
    if (cmbCliente->currentIndex() < 0)
        return;

    int idCliente = cmbCliente->currentData().toInt();
    QString codigoVendedor =
        consultaUnica("SELECT IFNULL(codigo_vendedor, '') FROM cliente WHERE id_cliente = " +
                      QString::number(idCliente)).toString();

    int posicion = cmbVendedor->findData(codigoVendedor);
    if (posicion >= 0)
        cmbVendedor->setCurrentIndex(posicion);
}

// El plazo solo se puede escribir cuando la factura es a credito
void NuevaFacturaDialog::condicionCambiada()
{
    spnPlazo->setEnabled(cmbCondicion->currentData().toString() == "CREDITO");
}

//  Agrega el producto elegido a la factura
void NuevaFacturaDialog::agregarProducto()
{
    if (cmbProducto->count() == 0) {
        QMessageBox::warning(this, "Sin productos", "No hay productos con existencia disponible.");
        return;
    }

    QString codigo = cmbProducto->currentData().toString();
    int cantidad = spnCantidad->value();

    // Datos actuales del producto (precio y existencia al momento de agregar)
    QSqlQuery consulta;
    consulta.exec("SELECT nombre, precio, existencia FROM producto WHERE codigo = " + sqlTexto(codigo));
    if (!consulta.next())
        return;
    QString nombre = consulta.value(0).toString();
    double precio  = consulta.value(1).toDouble();
    int existencia = consulta.value(2).toInt();

    // ¿Ya estaba en la factura? Entonces se suma la cantidad
    int posicion = -1;
    int yaAgregado = 0;
    for (int i = 0; i < lineas.size(); i++) {
        if (lineas[i].codigo == codigo) {
            posicion = i;
            yaAgregado = lineas[i].cantidad;
        }
    }

    if (yaAgregado + cantidad > existencia) {
        QMessageBox::warning(this, "Existencia insuficiente",
                             "Solo hay " + QString::number(existencia) + " unidades de " + nombre +
                             " en existencia" +
                             (yaAgregado > 0 ? " (ya agregaste " + QString::number(yaAgregado) + ")." : "."));
        return;
    }

    if (posicion >= 0) {
        lineas[posicion].cantidad += cantidad;
    } else {
        LineaFactura linea;
        linea.codigo = codigo;
        linea.nombre = nombre;
        linea.cantidad = cantidad;
        linea.precio = precio;
        lineas.append(linea);
    }

    spnCantidad->setValue(1);
    mostrarLineas();
}

void NuevaFacturaDialog::quitarLinea()
{
    int fila = tablaLineas->currentRow();
    if (fila < 0) {
        QMessageBox::information(this, "Quitar producto", "Selecciona primero el producto que deseas quitar.");
        return;
    }
    lineas.removeAt(fila);
    mostrarLineas();
}

//  Dibuja la tabla del detalle y calcula los totales
static QTableWidgetItem *celda(const QString &texto, bool derecha)
{
    QTableWidgetItem *item = new QTableWidgetItem(texto);
    if (derecha)
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}

void NuevaFacturaDialog::mostrarLineas()
{
    tablaLineas->setRowCount(lineas.size());

    double subtotal = 0;
    for (int i = 0; i < lineas.size(); i++) {
        double subLinea = std::round(lineas[i].cantidad * lineas[i].precio * 100) / 100.0;
        subtotal += subLinea;

        tablaLineas->setItem(i, 0, celda(lineas[i].codigo, false));
        tablaLineas->setItem(i, 1, celda(lineas[i].nombre, false));
        tablaLineas->setItem(i, 2, celda(QString::number(lineas[i].cantidad), true));
        tablaLineas->setItem(i, 3, celda(formatoQ(lineas[i].precio), true));
        tablaLineas->setItem(i, 4, celda(formatoQ(subLinea), true));
    }

    double iva   = std::round(subtotal * 0.12 * 100) / 100.0;
    double total = subtotal + iva;

    lblSubtotal->setText(formatoQ(subtotal));
    lblIva->setText(formatoQ(iva));
    lblTotal->setText(formatoQ(total));
}

//  GUARDAR: crea la factura y todas sus lineas en una sola operacion.
void NuevaFacturaDialog::guardar()
{
    if (cmbCliente->currentIndex() < 0) {
        QMessageBox::warning(this, "Datos incompletos", "Primero debes registrar al menos un cliente.");
        return;
    }
    if (cmbVendedor->currentIndex() < 0) {
        QMessageBox::warning(this, "Datos incompletos", "Selecciona el vendedor.");
        return;
    }
    if (lineas.isEmpty()) {
        QMessageBox::warning(this, "Factura vacía", "Agrega al menos un producto a la factura.");
        return;
    }

    QString idCliente = QString::number(cmbCliente->currentData().toInt());
    QString vendedor  = sqlTexto(cmbVendedor->currentData().toString());
    QString condicion = cmbCondicion->currentData().toString();
    int plazo = (condicion == "CREDITO") ? spnPlazo->value() : 0;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Error", "No se pudo iniciar el guardado de la factura.");
        return;
    }

    // 1) Encabezado de la factura
    QSqlQuery consulta;
    QString sqlFactura =
        "INSERT INTO factura (id_cliente, codigo_vendedor, condicion, plazo_dias) VALUES (" +
        idCliente + ", " + vendedor + ", " + sqlTexto(condicion) + ", " + QString::number(plazo) + ")";

    if (!consulta.exec(sqlFactura)) {
        QSqlError error = consulta.lastError();
        db.rollback();
        mostrarErrorBD(this, error);
        return;
    }
    int idFactura = consultaUnica("SELECT LAST_INSERT_ID()").toInt();

    // 2) Cada producto de la factura
    for (int i = 0; i < lineas.size(); i++) {
        QString sqlDetalle =
            "INSERT INTO detalle_factura (id_factura, codigo_producto, cantidad, precio_unitario) VALUES (" +
            QString::number(idFactura) + ", " + sqlTexto(lineas[i].codigo) + ", " +
            QString::number(lineas[i].cantidad) + ", " + QString::number(lineas[i].precio, 'f', 2) + ")";

        if (!consulta.exec(sqlDetalle)) {
            QSqlError error = consulta.lastError();
            db.rollback();                     // se deshace TODO: no queda factura a medias
            mostrarErrorBD(this, error);
            return;
        }
    }

    if (!db.commit()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo guardar la factura.");
        return;
    }

    // 3) Leer los datos finales calculados por la base de datos
    consulta.exec("SELECT serie, correlativo, total FROM factura WHERE id_factura = " +
                  QString::number(idFactura));
    consulta.next();
    QString numero = consulta.value(0).toString() + "-" + consulta.value(1).toString();
    double total = consulta.value(2).toDouble();

    QString mensaje = "Factura " + numero + " registrada correctamente.\nTotal: " + formatoQ(total);

    // 4) Una factura al contado tiene un solo pago: se registra de una vez
    if (condicion == "CONTADO") {
        QSqlQuery pago;
        if (pago.exec("INSERT INTO pago (id_factura, monto) VALUES (" +
                      QString::number(idFactura) + ", " + QString::number(total, 'f', 2) + ")")) {
            mensaje += "\nSe registró el pago al contado.";
        } else {
            mostrarErrorBD(this, pago.lastError());
        }
    }

    QMessageBox::information(this, "Factura registrada", mensaje);

    accept();   // cierra la ventana indicando que se guardo
}
