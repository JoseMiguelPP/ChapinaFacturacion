#include "pagosdialog.h"
#include "database.h"
#include "modelosql.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

PagosDialog::PagosDialog(QWidget *parent)
    : QDialog(parent), idFactura(0), saldo(0)
{
    setWindowTitle("Pagos");
    resize(1000, 740);

    //      Tabla de facturas
    modelo = new ModeloSql(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla->setSelectionMode(QAbstractItemView::SingleSelection);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);

    //      Historial de pagos
    modeloPagos = new ModeloSql(this);
    tablaPagos = new QTableView;
    tablaPagos->setModel(modeloPagos);
    tablaPagos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaPagos->setAlternatingRowColors(true);

    //      Registrar pago
    lblInfo = new QLabel;
    lblInfo->setStyleSheet("font-weight: bold;");

    spnMonto = new QDoubleSpinBox;
    spnMonto->setDecimals(2);
    spnMonto->setPrefix("Q ");

    btnRegistrar = new QPushButton("Registrar pago");

    QFormLayout *formulario = new QFormLayout;
    formulario->addRow(lblInfo);
    formulario->addRow("Monto a pagar:", spnMonto);

    QGroupBox *grupo = new QGroupBox("Registrar pago");
    grupo->setLayout(formulario);

    QPushButton *btnCerrar = new QPushButton("Cerrar");
    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addStretch();
    filaBotones->addWidget(btnRegistrar);
    filaBotones->addWidget(btnCerrar);

    //      Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addWidget(new QLabel("Facturas (el saldo es lo que falta por pagar):"));
    principal->addWidget(tabla, 3);
    principal->addWidget(new QLabel("Historial de pagos de la factura seleccionada:"));
    principal->addWidget(tablaPagos, 2);
    principal->addWidget(grupo);
    principal->addLayout(filaBotones);

    //      Conectar acciones
    connect(tabla, &QTableView::clicked, this, &PagosDialog::filaSeleccionada);
    connect(btnRegistrar, &QPushButton::clicked, this, &PagosDialog::registrarPago);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);

    cargarFacturas();
}

//  Lista de facturas activas con su saldo
void PagosDialog::cargarFacturas()
{
    modelo->setQuery("SELECT id_factura, CONCAT(serie, '-', correlativo), fecha, cliente, condicion, "
                     "total, pagado, saldo_pendiente, fecha_vencimiento "
                     "FROM v_facturas WHERE estado = 'ACTIVA' ORDER BY id_factura");

    modelo->setHeaderData(1, Qt::Horizontal, "No. factura");
    modelo->setHeaderData(2, Qt::Horizontal, "Fecha");
    modelo->setHeaderData(3, Qt::Horizontal, "Cliente");
    modelo->setHeaderData(4, Qt::Horizontal, "Condición");
    modelo->setHeaderData(5, Qt::Horizontal, "Total (Q)");
    modelo->setHeaderData(6, Qt::Horizontal, "Pagado (Q)");
    modelo->setHeaderData(7, Qt::Horizontal, "Saldo (Q)");
    modelo->setHeaderData(8, Qt::Horizontal, "Vence");

    tabla->setColumnHidden(0, true);
    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(false);
    tabla->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);   // Cliente

    limpiarSeleccion();
}

// Deja la ventana sin ninguna factura seleccionada
void PagosDialog::limpiarSeleccion()
{
    idFactura = 0;
    saldo = 0;
    modeloPagos->clear();
    spnMonto->setRange(0, 0);
    spnMonto->setEnabled(false);
    btnRegistrar->setEnabled(false);
    lblInfo->setText("Selecciona una factura de la lista para registrar un pago.");
}

//  Clic en una factura: muestra su saldo y su historial de pagos
void PagosDialog::filaSeleccionada(const QModelIndex &indice)
{
    int fila = indice.row();

    idFactura     = modelo->data(modelo->index(fila, 0)).toInt();
    numeroFactura = modelo->data(modelo->index(fila, 1)).toString();
    condicion     = modelo->data(modelo->index(fila, 4)).toString();

    // El saldo se lee directo de la base de datos para que sea exacto
    QSqlQuery consulta("SELECT total, pagado, saldo_pendiente FROM v_facturas WHERE id_factura = " +
                       QString::number(idFactura));
    consulta.next();
    double total  = consulta.value(0).toDouble();
    double pagado = consulta.value(1).toDouble();
    saldo         = consulta.value(2).toDouble();

    lblInfo->setText("Factura " + numeroFactura + "   |   Total: " + formatoQ(total) +
                     "   Pagado: " + formatoQ(pagado) + "   Saldo: " + formatoQ(saldo));

    if (saldo > 0) {
        spnMonto->setRange(0.01, saldo);
        spnMonto->setValue(saldo);
        spnMonto->setEnabled(true);
        btnRegistrar->setEnabled(true);
    } else {
        spnMonto->setRange(0, 0);
        spnMonto->setEnabled(false);
        btnRegistrar->setEnabled(false);
        lblInfo->setText(lblInfo->text() + "   (FACTURA PAGADA)");
    }

    cargarPagos();
}

void PagosDialog::cargarPagos()
{
    modeloPagos->setQuery("SELECT correlativo, fecha, monto, saldo FROM pago "
                          "WHERE id_factura = " + QString::number(idFactura) + " "
                          "ORDER BY correlativo");

    modeloPagos->setHeaderData(0, Qt::Horizontal, "No. de pago");
    modeloPagos->setHeaderData(1, Qt::Horizontal, "Fecha");
    modeloPagos->setHeaderData(2, Qt::Horizontal, "Monto (Q)");
    modeloPagos->setHeaderData(3, Qt::Horizontal, "Saldo (Q)");
    tablaPagos->resizeColumnsToContents();
    tablaPagos->horizontalHeader()->setStretchLastSection(false);
}

//  REGISTRAR el pago (la fecha es la de hoy)
void PagosDialog::registrarPago()
{
    if (idFactura == 0 || saldo <= 0)
        return;

    double monto = spnMonto->value();

    // Una factura al contado se paga completa, en un solo pago
    if (condicion == "CONTADO" && qAbs(monto - saldo) > 0.001) {
        QMessageBox::warning(this, "Factura al contado",
                             "Una factura al contado se paga completa en un solo pago (" +
                             formatoQ(saldo) + ").");
        spnMonto->setValue(saldo);
        return;
    }

    QSqlQuery consulta;
    if (!consulta.exec("INSERT INTO pago (id_factura, monto) VALUES (" +
                       QString::number(idFactura) + ", " + QString::number(monto, 'f', 2) + ")")) {
        mostrarErrorBD(this, consulta.lastError());
        return;
    }

    double nuevoSaldo = consultaUnica("SELECT saldo_pendiente FROM v_facturas WHERE id_factura = " +
                                      QString::number(idFactura)).toDouble();

    QMessageBox::information(this, "Pago registrado",
                             "Se registró el pago de " + formatoQ(monto) + " a la factura " +
                             numeroFactura + ".\nSaldo pendiente: " + formatoQ(nuevoSaldo));

    // Se vuelve a cargar la lista y se deja seleccionada la misma factura
    int idAnterior = idFactura;
    cargarFacturas();
    for (int fila = 0; fila < modelo->rowCount(); fila++) {
        if (modelo->data(modelo->index(fila, 0)).toInt() == idAnterior) {
            tabla->selectRow(fila);
            filaSeleccionada(modelo->index(fila, 0));
            break;
        }
    }
}
