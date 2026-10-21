#include "database.h"

#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

//  Busca en Windows el conector ODBC de MySQL
//  instalado, para no tener que escribir su nombre exacto.
//  Devuelve "" si no encuentra ninguno.
static QString buscarDriverOdbc()
{
    QStringList instalados;

#ifdef Q_OS_WIN
    // Windows guarda la lista de conectores ODBC instalados en el registro
    QSettings registro("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers",
                       QSettings::NativeFormat);
    instalados = registro.childKeys();
#endif

    // 1ro: el conector oficial de MySQL (version Unicode)
    for (const QString &nombre : instalados) {
        if (nombre.contains("MySQL") && nombre.contains("Unicode"))
            return nombre;
    }
    return "";
}

bool conectarBaseDeDatos(QString &mensajeError)
{
    QString driver = buscarDriverOdbc();
    if (driver.isEmpty()) {
        mensajeError = "No se encontró el conector ODBC de MySQL en esta computadora.\n\n"
                       "Instala \"MySQL Connector/ODBC\" (64 bits) desde:\n"
                       "https://dev.mysql.com/downloads/connector/odbc/\n"
                       "y vuelve a ejecutar el programa.";
        return false;
    }

    // Qt usa el driver QODBC, que a su vez usa el conector de MySQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    // La cadena se arma con join(";") para que NO quede un ";" sobrante al final
    // (algunos conectores ODBC fallan si la cadena termina en ";")
    QStringList partes;
    partes << "DRIVER={" + driver + "}"
           << "SERVER=" + BD_HOST
           << "PORT=" + QString::number(BD_PUERTO)
           << "DATABASE=" + BD_NOMBRE
           << "UID=" + BD_USUARIO
           << "PWD=" + BD_CLAVE;
    QString cadenaConexion = partes.join(";");
    db.setDatabaseName(cadenaConexion);

    if (!db.open()) {
        mensajeError = "Conector usado: " + driver + "\n\n" + db.lastError().text() +
                       "\n\nRevisa que el servidor MySQL esté encendido, que hayas ejecutado "
                       "ChapinaDB.sql y que el usuario y la clave de database.h sean correctos.";
        return false;
    }
    return true;
}

//  Protege un texto para meterlo en una instruccion SQL
QString sqlTexto(const QString &texto)
{
    QString limpio = texto;
    limpio.remove(QChar(0));              // caracteres nulos no permitidos
    limpio.replace("\\", "\\\\");       // cada barra invertida se duplica
    limpio.replace("'", "''");            // cada comilla simple se duplica
    return "'" + limpio + "'";
}

//  Deja solo el mensaje util de un error de la base de datos
//  Ej: "[MySQL][ODBC 8.0 Driver][mysqld]Existencia insuficiente" -> "Existencia insuficiente"
QString mensajeDeError(const QSqlError &error)
{
    QString texto = error.databaseText();
    int posicion = texto.lastIndexOf(']');
    if (posicion >= 0)
        texto = texto.mid(posicion + 1);
    return texto.trimmed();
}

//  Ventana de error de la base de datos (con mensajes amigables)
void mostrarErrorBD(QWidget *padre, const QSqlError &error,
                    const QString &mensajeDuplicado, const QString &mensajeRelacion)
{
    QString codigo = error.nativeErrorCode();
    QString mensaje;

    if (codigo == "1062" && !mensajeDuplicado.isEmpty())        // dato duplicado
        mensaje = mensajeDuplicado;
    else if (codigo == "1451" && !mensajeRelacion.isEmpty())    // tiene datos relacionados
        mensaje = mensajeRelacion;
    else
        mensaje = mensajeDeError(error);

    QMessageBox::critical(padre, "Error", mensaje);
}

//  Primera columna de la primera fila de un SELECT
QVariant consultaUnica(const QString &sql)
{
    QSqlQuery consulta;
    if (consulta.exec(sql) && consulta.next())
        return consulta.value(0);
    return QVariant();
}

//  Formato de dinero: Q1,234.50
QString formatoQ(double valor)
{
    return "Q" + QLocale(QLocale::English).toString(valor, 'f', 2);
}
