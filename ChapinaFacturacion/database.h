#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QWidget>

// ---------------------------------------------------------------
//  Datos de conexion a MySQL. Si tu servidor es distinto,
//  cambialos SOLO aqui.
// ---------------------------------------------------------------
const QString BD_HOST    = "localhost";
const int     BD_PUERTO  = 3306;
const QString BD_NOMBRE  = "chapina";
const QString BD_USUARIO = "chapina_app";
const QString BD_CLAVE   = "chapina123";

// Abre la conexion con la base de datos.
// Devuelve true si todo salio bien; si falla, deja el motivo en mensajeError.
bool conectarBaseDeDatos(QString &mensajeError);

// Convierte un texto en un valor seguro para escribirlo dentro de una
// instruccion SQL (le pone comillas y protege los caracteres especiales).
//   sqlTexto("Ana")     ->  'Ana'
//   sqlTexto("O'Brien") ->  'O''Brien'
// REGLA: todo texto que escriba el usuario debe pasar por sqlTexto().
QString sqlTexto(const QString &texto);

// Quita de un error de la base de datos la parte tecnica del conector
// y deja solo el mensaje, para mostrarlo al usuario.
QString mensajeDeError(const QSqlError &error);

// Muestra una ventana con el error de la base de datos.
//  mensajeDuplicado: texto a mostrar si el dato ya existe (llave repetida)
//  mensajeRelacion : texto a mostrar si no se puede borrar porque tiene datos relacionados
void mostrarErrorBD(QWidget *padre, const QSqlError &error,
                    const QString &mensajeDuplicado = "",
                    const QString &mensajeRelacion = "");

// Ejecuta un SELECT y devuelve la primera columna de la primera fila
// (sirve para COUNT, SUM, un nombre, etc.). Si no hay datos devuelve un QVariant vacio.
QVariant consultaUnica(const QString &sql);

// Da formato de dinero:  1234.5  ->  "Q1,234.50"
QString formatoQ(double valor);

#endif // DATABASE_H
