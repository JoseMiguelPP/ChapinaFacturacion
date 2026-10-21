#include "reportesdialog.h"
#include "modelosql.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

ReportesDialog::ReportesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Reportes");
    resize(1000, 650);

    // Definicion de los 6 reportes
    // (cada uno lee de una vista creada en ChapinaDB.sql)
    reportes.append({"Reporte de Clientes",
        "SELECT nombre, nit, telefono, direccion, persona_contacto, email, pagina_web, vendedor "
        "FROM v_reporte_clientes",
        {"Nombre", "NIT", "Teléfono", "Dirección", "Persona de contacto", "E-mail", "Página web", "Vendedor"}});

    reportes.append({"Reporte de Vendedores",
        "SELECT codigo_empleado, dpi, nombre, telefono, direccion, email, num_clientes "
        "FROM v_reporte_vendedores",
        {"Código", "DPI", "Nombre", "Teléfono", "Dirección", "E-mail", "No. de clientes"}});

    reportes.append({"Reporte de Productos Vendidos",
        "SELECT codigo, nombre, unidades_vendidas, monto_vendido FROM v_productos_vendidos",
        {"Código", "Producto", "Unidades vendidas", "Monto vendido (Q)"}});

    reportes.append({"Listado de Ventas (Facturas del Día)",
        "SELECT CONCAT(serie, '-', correlativo), fecha, cliente, vendedor, condicion, "
        "subtotal, iva, total FROM v_ventas_del_dia",
        {"No. factura", "Fecha", "Cliente", "Vendedor", "Condición", "Subtotal (Q)", "IVA (Q)", "Total (Q)"}});

    reportes.append({"Reporte de Pagos del Día",
        "SELECT CONCAT(serie, '-', factura), fecha, cliente, no_pago, monto, saldo FROM v_pagos_del_dia",
        {"Factura", "Fecha", "Cliente", "No. de pago", "Monto (Q)", "Saldo (Q)"}});

    reportes.append({"Reporte de Inventario Existente",
        "SELECT codigo, nombre, costo, precio, existencia FROM v_inventario",
        {"Código", "Producto", "Costo (Q)", "Precio (Q)", "Existencia"}});

    //      Elementos visuales
    cmbReporte = new QComboBox;
    for (int i = 0; i < reportes.size(); i++)
        cmbReporte->addItem(reportes[i].titulo);

    QHBoxLayout *filaSuperior = new QHBoxLayout;
    filaSuperior->addWidget(new QLabel("Reporte:"));
    filaSuperior->addWidget(cmbReporte, 1);

    modelo = new ModeloSql(this);
    tabla = new QTableView;
    tabla->setModel(modelo);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setAlternatingRowColors(true);

    QPushButton *btnCerrar = new QPushButton("Cerrar");
    QHBoxLayout *filaInferior = new QHBoxLayout;
    filaInferior->addStretch();
    filaInferior->addWidget(btnCerrar);

    QVBoxLayout *principal = new QVBoxLayout(this);
    principal->addLayout(filaSuperior);
    principal->addWidget(tabla, 1);
    principal->addLayout(filaInferior);

    connect(cmbReporte, &QComboBox::currentIndexChanged, this, &ReportesDialog::mostrarReporte);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);

    mostrarReporte();
}

//  Muestra el reporte elegido en la lista
void ReportesDialog::mostrarReporte()
{
    int posicion = cmbReporte->currentIndex();
    if (posicion < 0)
        return;
    const Reporte &reporte = reportes[posicion];

    modelo->setQuery(reporte.sql);

    for (int i = 0; i < reporte.encabezados.size(); i++)
        modelo->setHeaderData(i, Qt::Horizontal, reporte.encabezados[i]);

    tabla->resizeColumnsToContents();
    tabla->horizontalHeader()->setStretchLastSection(false);
}
