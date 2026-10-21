#include <QApplication>
#include <QMessageBox>

#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1. Conectar con la base de datos antes de mostrar nada
    QString error;
    if (!conectarBaseDeDatos(error)) {
        QMessageBox::critical(nullptr, "Error de conexión",
                              "No se pudo conectar con la base de datos.\n\n" + error);
        return 1;
    }

    MainWindow ventana;
    ventana.show();

    return app.exec();
}
