#include "facturasdialog.h"
#include "database.h"
#include "modelosql.h"
#include "nuevafacturadialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

FacturasDialog::FacturasDialog(QWidget *parent)
    : QDialog(parent), idSeleccionada(0)
{
    setWindowTitle("Facturación");
    resize(1100, 720);

    //          Tabla de facturas
    modelo = new ModeloSql(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla->setSelectionMode(QAbstractItemView::SingleSelection);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);

    //          Tabla del detalle
    modeloDetalle = new ModeloSql(this);
    tablaDetalle = new QTableView;
    tablaDetalle->setModel(modeloDetalle);
    tablaDetalle->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaDetalle->setAlternatingRowColors(true);

    lblResumen = new QLabel;
    lblResumen->setStyleSheet("font-size: 14px; font-weight: bold;");

    //          Botones
    QPushButton *btnNueva = new QPushButton("Nueva factura");
    btnAnular = new QPushButton("Anular factura");
    btnAnular->setEnabled(false);
    QPushButton *btnCerrar = new QPushButton("Cerrar");

    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addWidget(btnNueva);
    filaBotones->addWidget(btnAnular);
    filaBotones->addStretch();
    filaBotones->addWidget(btnCerrar);

    //          Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(tabla, 3);
    principal->addWidget(new QLabel("Detalle de la factura seleccionada:"));
    principal->addWidget(tablaDetalle, 2);
    principal->addWidget(lblResumen);
    principal->addLayout(filaBotones);

    //          Conectar acciones
    connect(btnNueva,  &QPushButton::clicked, this, &FacturasDialog::nuevaFactura);
    connect(btnAnular, &QPushButton::clicked, this, &FacturasDialog::anularFactura);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);
    connect(tabla, &QTableView::clicked, this, &FacturasDialog::filaSeleccionada);

    cargarTabla();
}

//  CONSULTAR facturas
//  Columnas: 0 id (oculta), 1 numero, 2 fecha, 3 cliente, 4 vendedor,
//            5 condicion, 6 total, 7 vence, 8 estado
void FacturasDialog::cargarTabla()
{
    modelo->setQuery("SELECT id_factura, CONCAT(serie, '-', correlativo), fecha, cliente, vendedor, "
                     "condicion, total, fecha_vencimiento, estado "
                     "FROM v_facturas ORDER BY id_factura DESC");

    modelo->setHeaderData(1, Qt::Horizontal, "No. factura");
    modelo->setHeaderData(2, Qt::Horizontal, "Fecha");
    modelo->setHeaderData(3, Qt::Horizontal, "Cliente");
    modelo->setHeaderData(4, Qt::Horizontal, "Vendedor");
    modelo->setHeaderData(5, Qt::Horizontal, "Condición");
    modelo->setHeaderData(6, Qt::Horizontal, "Total (Q)");
    modelo->setHeaderData(7, Qt::Horizontal, "Vence");
    modelo->setHeaderData(8, Qt::Horizontal, "Estado");

    tabla->setColumnHidden(0, true);
    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(false);
    tabla->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);   // Cliente

    // Al recargar no queda nada seleccionado
    idSeleccionada = 0;
    btnAnular->setEnabled(false);
    modeloDetalle->clear();
    lblResumen->clear();
}

//  Clic en una factura: muestra su detalle
void FacturasDialog::filaSeleccionada(const QModelIndex &indice)
{
    int fila = indice.row();

    idSeleccionada     = modelo->data(modelo->index(fila, 0)).toInt();
    numeroSeleccionado = modelo->data(modelo->index(fila, 1)).toString();
    estadoSeleccionado = modelo->data(modelo->index(fila, 8)).toString();

    btnAnular->setEnabled(estadoSeleccionado == "ACTIVA");
    cargarDetalle();
}

void FacturasDialog::cargarDetalle()
{
    modeloDetalle->setQuery("SELECT d.codigo_producto, p.nombre, d.cantidad, d.precio_unitario, d.subtotal "
                            "FROM detalle_factura d "
                            "JOIN producto p ON p.codigo = d.codigo_producto "
                            "WHERE d.id_factura = " + QString::number(idSeleccionada) + " "
                            "ORDER BY d.id_detalle");

    modeloDetalle->setHeaderData(0, Qt::Horizontal, "Código");
    modeloDetalle->setHeaderData(1, Qt::Horizontal, "Producto");
    modeloDetalle->setHeaderData(2, Qt::Horizontal, "Cantidad");
    modeloDetalle->setHeaderData(3, Qt::Horizontal, "Precio (Q)");
    modeloDetalle->setHeaderData(4, Qt::Horizontal, "Subtotal (Q)");
    tablaDetalle->resizeColumnsToContents();
    tablaDetalle->horizontalHeader()->setStretchLastSection(false);
    tablaDetalle->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);   // Producto

    // Subtotal, IVA y total de la factura
    QSqlQuery consulta("SELECT subtotal, iva, total FROM factura WHERE id_factura = " +
                       QString::number(idSeleccionada));
    if (consulta.next()) {
        lblResumen->setText("Factura " + numeroSeleccionado + "    |    Subtotal: " +
                            formatoQ(consulta.value(0).toDouble()) + "    IVA: " +
                            formatoQ(consulta.value(1).toDouble()) + "    Total: " +
                            formatoQ(consulta.value(2).toDouble()));
    }
}

//  NUEVA factura
void FacturasDialog::nuevaFactura()
{
    if (consultaUnica("SELECT COUNT(*) FROM cliente").toInt() == 0) {
        QMessageBox::warning(this, "Sin clientes", "Primero debes registrar al menos un cliente.");
        return;
    }
    if (consultaUnica("SELECT COUNT(*) FROM producto WHERE existencia > 0").toInt() == 0) {
        QMessageBox::warning(this, "Sin productos", "No hay productos con existencia para facturar.");
        return;
    }

    NuevaFacturaDialog ventana(this);
    if (ventana.exec() == QDialog::Accepted)
        cargarTabla();
}

//  ANULAR factura (los productos regresan al inventario)
void FacturasDialog::anularFactura()
{
    if (idSeleccionada == 0 || estadoSeleccionado != "ACTIVA")
        return;

    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this, "Confirmar anulación",
        "¿Seguro que deseas anular la factura " + numeroSeleccionado + "?\n\n"
        "Los productos regresarán al inventario.");
    if (respuesta != QMessageBox::Yes)
        return;

    QSqlQuery consulta;
    if (!consulta.exec("UPDATE factura SET estado = 'ANULADA' WHERE id_factura = " +
                       QString::number(idSeleccionada) + " AND estado = 'ACTIVA'")) {
        mostrarErrorBD(this, consulta.lastError());
        return;
    }

    QMessageBox::information(this, "Listo", "La factura " + numeroSeleccionado + " fue anulada.");
    cargarTabla();
}
