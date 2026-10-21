#include "mainwindow.h"
#include "clientesdialog.h"
#include "facturasdialog.h"
#include "pagosdialog.h"
#include "productosdialog.h"
#include "reportesdialog.h"
#include "vendedoresdialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Sistema de Facturación - Representaciones Chapina");
    resize(720, 460);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    //   Titulos
    QLabel *titulo = new QLabel("Representaciones “Chapina”, C.A.");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 26px; font-weight: bold; color: #1b5e20;");

    QLabel *subtitulo = new QLabel("Sistema de Facturación");
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setStyleSheet("font-size: 16px; color: #555555;");

    //   Botones de los modulos
    QStringList modulos = {"Vendedores", "Clientes", "Productos",
                           "Facturación", "Pagos", "Reportes"};

    QGridLayout *cuadricula = new QGridLayout;
    cuadricula->setSpacing(15);

    for (int i = 0; i < modulos.size(); i++) {
        QString nombre = modulos[i];
        QPushButton *boton = new QPushButton(nombre);
        cuadricula->addWidget(boton, i / 3, i % 3);   // fila = i/3, columna = i%3

        connect(boton, &QPushButton::clicked, this, [this, nombre]() {
            abrirModulo(nombre);
        });
    }

    //   Armar la ventana
    QVBoxLayout *principal = new QVBoxLayout(central);
    principal->setContentsMargins(30, 25, 30, 25);
    principal->addStretch(1);
    principal->addWidget(titulo);
    principal->addWidget(subtitulo);
    principal->addSpacing(25);
    principal->addLayout(cuadricula);
    principal->addStretch(1);

    // Estilo de los botones
    central->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-size: 17px;"
        "              border-radius: 8px; padding: 22px; }"
        "QPushButton:hover   { background-color: #388e3c; }"
        "QPushButton:pressed { background-color: #1b5e20; }");

}

void MainWindow::abrirModulo(const QString &nombre)
{
    if (nombre == "Vendedores") {
        VendedoresDialog ventana(this);
        ventana.exec();
    } else if (nombre == "Clientes") {
        ClientesDialog ventana(this);
        ventana.exec();
    } else if (nombre == "Productos") {
        ProductosDialog ventana(this);
        ventana.exec();
    } else if (nombre == "Facturación") {
        FacturasDialog ventana(this);
        ventana.exec();
    } else if (nombre == "Pagos") {
        PagosDialog ventana(this);
        ventana.exec();
    } else if (nombre == "Reportes") {
        ReportesDialog ventana(this);
        ventana.exec();
    }
}
