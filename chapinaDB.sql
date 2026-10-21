DROP DATABASE IF EXISTS chapina;
CREATE DATABASE chapina CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE chapina;

CREATE TABLE vendedor (
    codigo_empleado VARCHAR(10)  NOT NULL PRIMARY KEY,
    dpi             VARCHAR(13)  NOT NULL UNIQUE,
    nombre          VARCHAR(100) NOT NULL,
    telefono        VARCHAR(20),
    direccion       VARCHAR(150),
    email           VARCHAR(100)
) ENGINE=InnoDB;

CREATE TABLE cliente (
    id_cliente       INT AUTO_INCREMENT PRIMARY KEY,
    nombre           VARCHAR(100) NOT NULL,
    nit              VARCHAR(15)  NOT NULL UNIQUE,
    telefono         VARCHAR(20),
    direccion        VARCHAR(150),
    persona_contacto VARCHAR(100),
    email            VARCHAR(100),
    pagina_web       VARCHAR(100),
    codigo_vendedor  VARCHAR(10),
    FOREIGN KEY (codigo_vendedor) REFERENCES vendedor(codigo_empleado)
        ON UPDATE CASCADE ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE producto (
    codigo     VARCHAR(10)   NOT NULL PRIMARY KEY,
    nombre     VARCHAR(100)  NOT NULL,
    costo      DECIMAL(10,2) NOT NULL CHECK (costo >= 0),
    precio     DECIMAL(10,2) NOT NULL CHECK (precio >= 0),
    existencia INT           NOT NULL DEFAULT 0 CHECK (existencia >= 0)
) ENGINE=InnoDB;

CREATE TABLE factura (
    id_factura        INT AUTO_INCREMENT PRIMARY KEY,
    serie             VARCHAR(5)    NOT NULL DEFAULT 'A',
    correlativo       INT NULL, 
    id_cliente        INT           NOT NULL,
    codigo_vendedor   VARCHAR(10)   NOT NULL,
    fecha             DATE          NOT NULL DEFAULT (CURDATE()),
    subtotal          DECIMAL(10,2) NOT NULL DEFAULT 0,
    iva               DECIMAL(10,2) NOT NULL DEFAULT 0,
    total             DECIMAL(10,2) NOT NULL DEFAULT 0,
    condicion         ENUM('CONTADO','CREDITO') NOT NULL,
    plazo_dias        INT           NOT NULL DEFAULT 0,
    fecha_vencimiento DATE NULL,
    estado            ENUM('ACTIVA','ANULADA') NOT NULL DEFAULT 'ACTIVA',
    UNIQUE (serie, correlativo),
    CHECK (condicion = 'CONTADO' OR plazo_dias > 0),
    FOREIGN KEY (id_cliente)      REFERENCES cliente(id_cliente),
    FOREIGN KEY (codigo_vendedor) REFERENCES vendedor(codigo_empleado)
        ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE detalle_factura (
    id_detalle      INT AUTO_INCREMENT PRIMARY KEY,
    id_factura      INT           NOT NULL,
    codigo_producto VARCHAR(10)   NOT NULL,
    cantidad        INT           NOT NULL CHECK (cantidad > 0),
    precio_unitario DECIMAL(10,2) NOT NULL CHECK (precio_unitario >= 0),
    subtotal        DECIMAL(10,2) NOT NULL DEFAULT 0,
    FOREIGN KEY (id_factura)      REFERENCES factura(id_factura) ON DELETE CASCADE,
    FOREIGN KEY (codigo_producto) REFERENCES producto(codigo) ON UPDATE CASCADE
) ENGINE=InnoDB;

CREATE TABLE pago (
    id_pago     INT AUTO_INCREMENT PRIMARY KEY,
    id_factura  INT           NOT NULL,
    correlativo INT NULL,
    fecha       DATE          NOT NULL DEFAULT (CURDATE()),
    monto       DECIMAL(10,2) NOT NULL CHECK (monto > 0),
    saldo       DECIMAL(10,2) NULL,
    FOREIGN KEY (id_factura) REFERENCES factura(id_factura)
) ENGINE=InnoDB;

DELIMITER $$

CREATE TRIGGER trg_factura_bi BEFORE INSERT ON factura
FOR EACH ROW
BEGIN
    SET NEW.correlativo = (SELECT IFNULL(MAX(correlativo), 0) + 1
                           FROM factura WHERE serie = NEW.serie);
    IF NEW.condicion = 'CREDITO' THEN
        SET NEW.fecha_vencimiento = DATE_ADD(NEW.fecha, INTERVAL NEW.plazo_dias DAY);
    ELSE
        SET NEW.fecha_vencimiento = NULL;
    END IF;
END$$


CREATE TRIGGER trg_detalle_bi BEFORE INSERT ON detalle_factura
FOR EACH ROW
BEGIN
    DECLARE v_existencia INT;
    DECLARE v_estado VARCHAR(10);

    SELECT estado INTO v_estado FROM factura WHERE id_factura = NEW.id_factura;
    IF v_estado = 'ANULADA' THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'La factura esta anulada';
    END IF;

    SELECT existencia INTO v_existencia FROM producto
     WHERE codigo = NEW.codigo_producto;
    IF v_existencia < NEW.cantidad THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Existencia insuficiente para el producto';
    END IF;

    SET NEW.subtotal = ROUND(NEW.cantidad * NEW.precio_unitario, 2);
END$$

CREATE TRIGGER trg_detalle_ai AFTER INSERT ON detalle_factura
FOR EACH ROW
BEGIN
    DECLARE v_sub DECIMAL(10,2);

    UPDATE producto
       SET existencia = existencia - NEW.cantidad
     WHERE codigo = NEW.codigo_producto;

    SELECT SUM(subtotal) INTO v_sub
      FROM detalle_factura WHERE id_factura = NEW.id_factura;

    UPDATE factura
       SET subtotal = v_sub,
           iva      = ROUND(v_sub * 0.12, 2),
           total    = v_sub + ROUND(v_sub * 0.12, 2)
     WHERE id_factura = NEW.id_factura;
END$$

CREATE TRIGGER trg_factura_au AFTER UPDATE ON factura
FOR EACH ROW
BEGIN
    IF OLD.estado = 'ACTIVA' AND NEW.estado = 'ANULADA' THEN
        UPDATE producto p
          JOIN (SELECT codigo_producto, SUM(cantidad) AS cant
                  FROM detalle_factura
                 WHERE id_factura = NEW.id_factura
                 GROUP BY codigo_producto) d
            ON d.codigo_producto = p.codigo
           SET p.existencia = p.existencia + d.cant;
    END IF;
END$$

CREATE TRIGGER trg_pago_bi BEFORE INSERT ON pago
FOR EACH ROW
BEGIN
    DECLARE v_estado    VARCHAR(10);
    DECLARE v_condicion VARCHAR(10);
    DECLARE v_total     DECIMAL(10,2);
    DECLARE v_pagado    DECIMAL(10,2);
    DECLARE v_num       INT;

    SELECT estado, condicion, total
      INTO v_estado, v_condicion, v_total
      FROM factura WHERE id_factura = NEW.id_factura;

    SELECT IFNULL(SUM(monto), 0), COUNT(*)
      INTO v_pagado, v_num
      FROM pago WHERE id_factura = NEW.id_factura;

    IF v_estado = 'ANULADA' THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'No se puede pagar una factura anulada';
    END IF;
    IF v_condicion = 'CONTADO' AND v_num > 0 THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'Una factura al contado solo puede tener un pago';
    END IF;
    IF NEW.monto > v_total - v_pagado THEN
        SIGNAL SQLSTATE '45000'
            SET MESSAGE_TEXT = 'El monto excede el saldo pendiente de la factura';
    END IF;

    SET NEW.correlativo = v_num + 1;
    SET NEW.saldo = v_total - v_pagado - NEW.monto;
END$$

DELIMITER ;

-- VISTAS
-- Reporte de Clientes
CREATE VIEW v_reporte_clientes AS
SELECT c.id_cliente, c.nombre, c.nit, c.telefono, c.direccion,
       c.persona_contacto, c.email, c.pagina_web,
       IFNULL(v.nombre, '(sin vendedor)') AS vendedor
FROM cliente c
LEFT JOIN vendedor v ON v.codigo_empleado = c.codigo_vendedor
ORDER BY c.nombre;

-- Reporte de Vendedores
CREATE VIEW v_reporte_vendedores AS
SELECT v.codigo_empleado, v.dpi, v.nombre, v.telefono, v.direccion, v.email,
       (SELECT COUNT(*) FROM cliente c
         WHERE c.codigo_vendedor = v.codigo_empleado) AS num_clientes
FROM vendedor v
ORDER BY v.nombre;

-- Reporte de Productos Vendidos
CREATE VIEW v_productos_vendidos AS
SELECT p.codigo, p.nombre,
       SUM(d.cantidad) AS unidades_vendidas,
       SUM(d.subtotal) AS monto_vendido
FROM detalle_factura d
JOIN factura  f ON f.id_factura = d.id_factura AND f.estado = 'ACTIVA'
JOIN producto p ON p.codigo = d.codigo_producto
GROUP BY p.codigo, p.nombre
ORDER BY unidades_vendidas DESC;

-- Facturas con pagado y saldo pendiente
CREATE VIEW v_facturas AS
SELECT f.id_factura, f.serie, f.correlativo, f.fecha,
       c.nombre AS cliente, c.nit,
       v.nombre AS vendedor,
       f.subtotal, f.iva, f.total,
       f.condicion, f.plazo_dias, f.fecha_vencimiento, f.estado,
       IFNULL((SELECT SUM(p.monto) FROM pago p
                WHERE p.id_factura = f.id_factura), 0) AS pagado,
       f.total - IFNULL((SELECT SUM(p.monto) FROM pago p
                          WHERE p.id_factura = f.id_factura), 0) AS saldo_pendiente
FROM factura f
JOIN cliente  c ON c.id_cliente = f.id_cliente
JOIN vendedor v ON v.codigo_empleado = f.codigo_vendedor;

-- Listado de Ventas (Facturas del Dia)
CREATE VIEW v_ventas_del_dia AS
SELECT * FROM v_facturas
WHERE fecha = CURDATE() AND estado = 'ACTIVA'
ORDER BY correlativo;

-- Reporte de Pagos del Dia
CREATE VIEW v_pagos_del_dia AS
SELECT p.id_pago, p.fecha, f.serie, f.correlativo AS factura,
       c.nombre AS cliente, p.correlativo AS no_pago, p.monto, p.saldo
FROM pago p
JOIN factura f ON f.id_factura = p.id_factura
JOIN cliente c ON c.id_cliente = f.id_cliente
WHERE p.fecha = CURDATE()
ORDER BY p.id_pago;

-- Reporte de Inventario Existente
CREATE VIEW v_inventario AS
SELECT codigo, nombre, costo, precio, existencia,
       costo * existencia AS valor_inventario
FROM producto
WHERE existencia > 0
ORDER BY nombre;

CREATE USER IF NOT EXISTS 'chapina_app'@'localhost' IDENTIFIED BY 'chapina123';
GRANT ALL PRIVILEGES ON chapina.* TO 'chapina_app'@'localhost';
FLUSH PRIVILEGES;

USE chapina;

INSERT INTO vendedor (codigo_empleado, dpi, nombre, telefono, direccion, email) VALUES
('V001', '2456789012301', 'Carlos Méndez',  '5512-3401', 'Zona 7, Ciudad de Guatemala',  'cmendez@chapina.com'),
('V002', '2987654321011', 'Ana López',      '5523-8890', 'Zona 11, Ciudad de Guatemala', 'alopez@chapina.com'),
('V003', '1765432109802', 'Luis Hernández', '4120-7765', 'Mixco, Guatemala',             'lhernandez@chapina.com');

INSERT INTO cliente (nombre, nit, telefono, direccion, persona_contacto, email, pagina_web, codigo_vendedor) VALUES
('Tienda La Esperanza',   '1234567-8', '2250-1111', 'Zona 1, Guatemala',  'María Pérez',    'esperanza@correo.com', NULL,                 'V001'),
('Abarrotes El Sol',      '2345678-9', '2250-2222', 'Zona 5, Guatemala',  'José Ramírez',   'elsol@correo.com',     'www.elsol.com.gt',   'V001'),
('Minimarket San José',   '3456789-0', '2250-3333', 'Villa Nueva',        'Rosa Castillo',  'sanjose@correo.com',   NULL,                 'V002'),
('Supermercado Don Beto', '4567890-1', '2250-4444', 'Zona 18, Guatemala', 'Alberto Cruz',   'donbeto@correo.com',   'www.donbeto.com.gt', 'V002'),
('Despensa Familiar',     '5678901-2', '2250-5555', 'Mixco, Guatemala',   'Sandra Morales', 'despensa@correo.com',  NULL,                 'V003');

INSERT INTO producto (codigo, nombre, costo, precio, existencia) VALUES
('P001', 'Cloro 1 galón',                12.00, 18.50, 100),
('P002', 'Desinfectante lavanda 1 gal',  15.00, 22.00,  80),
('P003', 'Detergente en polvo 1 kg',     10.00, 15.00, 120),
('P004', 'Jabón de platos 750 ml',        8.00, 12.50,  90),
('P005', 'Escoba plástica',              14.00, 21.00,  50),
('P006', 'Trapeador de algodón',         16.00, 24.00,  40),
('P007', 'Papel higiénico (paquete 12)', 20.00, 29.00,  60),
('P008', 'Bolsas de basura (paq. 10)',    6.00,  9.50,  70),
('P009', 'Suavizante 2 L',               13.00, 19.00,  55),
('P010', 'Esponja multiuso (paq. 3)',     4.00,  7.00,   0);

-- Factura 1: contado, hoy
INSERT INTO factura (id_cliente, codigo_vendedor, condicion) VALUES (1, 'V001', 'CONTADO');
INSERT INTO detalle_factura (id_factura, codigo_producto, cantidad, precio_unitario) VALUES
(1, 'P001', 5, 18.50), (1, 'P003', 10, 15.00);

-- Factura 2: crédito 30 días, hace 10 días
INSERT INTO factura (id_cliente, codigo_vendedor, fecha, condicion, plazo_dias)
VALUES (3, 'V002', DATE_SUB(CURDATE(), INTERVAL 10 DAY), 'CREDITO', 30);
INSERT INTO detalle_factura (id_factura, codigo_producto, cantidad, precio_unitario) VALUES
(2, 'P005', 6, 21.00), (2, 'P006', 4, 24.00), (2, 'P007', 10, 29.00);

-- Factura 3: crédito 15 días, hoy
INSERT INTO factura (id_cliente, codigo_vendedor, condicion, plazo_dias)
VALUES (4, 'V002', 'CREDITO', 15);
INSERT INTO detalle_factura (id_factura, codigo_producto, cantidad, precio_unitario) VALUES
(3, 'P002', 8, 22.00), (3, 'P004', 12, 12.50);

-- Pagos
INSERT INTO pago (id_factura, monto) SELECT 1, total FROM factura WHERE id_factura = 1;           
INSERT INTO pago (id_factura, fecha, monto) VALUES (2, DATE_SUB(CURDATE(), INTERVAL 5 DAY), 200.00);
INSERT INTO pago (id_factura, monto) VALUES (2, 150.00);                                             
