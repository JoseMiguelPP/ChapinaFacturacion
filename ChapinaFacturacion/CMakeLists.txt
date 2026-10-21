cmake_minimum_required(VERSION 3.16)

project(ChapinaFacturacion VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt genera automaticamente el codigo de senales/slots (moc)
set(CMAKE_AUTOMOC ON)

# Qt6: Widgets = ventanas y botones | Sql = conexion a MySQL
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(ChapinaFacturacion
    main.cpp
    database.h
    database.cpp
    mainwindow.h
    mainwindow.cpp
    modelosql.h
    modelosql.cpp
    vendedoresdialog.h
    vendedoresdialog.cpp
    productosdialog.h
    productosdialog.cpp
    clientesdialog.h
    clientesdialog.cpp
    nuevafacturadialog.h
    nuevafacturadialog.cpp
    facturasdialog.h
    facturasdialog.cpp
    pagosdialog.h
    pagosdialog.cpp
    reportesdialog.h
    reportesdialog.cpp
)

target_link_libraries(ChapinaFacturacion PRIVATE Qt6::Widgets Qt6::Sql)

# Para que los acentos se lean bien con el compilador de Visual Studio
if(MSVC)
    target_compile_options(ChapinaFacturacion PRIVATE /utf-8)
endif()

# En Windows: no abrir ventana de consola negra junto con la aplicacion
set_target_properties(ChapinaFacturacion PROPERTIES WIN32_EXECUTABLE ON)

