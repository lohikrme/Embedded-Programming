BEGIN;
CREATE TABLE  AmbientData(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  temperature NUMERIC,
  humidity NUMERIC,
  pressure NUMERIC,
  device TEXT,
  loc TEXT,
  address TEXT,
  timerecord TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);
COMMIT;

INSERT INTO AmbientData(temperature, humidity, pressure, device, loc, address ) VALUES (28.3, 34.5, 991.17, "Pico1", "Forest_SE2", "TT10");
INSERT INTO AmbientData(temperature, humidity, pressure, device, loc, address ) VALUES (28.6, 35.5, 993.88, "Pico1", "Forest_SE2", "TT10");
INSERT INTO AmbientData(temperature, humidity, pressure, device, loc, address ) VALUES (28.9, 36.2, 994.67, "Pico1", "Forest_SE2", "TT10");
INSERT INTO AmbientData(temperature, humidity, pressure, device, loc, address ) VALUES (28.7, 36.8, 996.13, "Pico1", "Forest_SE2", "TT10");

SELECT * FROM AmbientData;