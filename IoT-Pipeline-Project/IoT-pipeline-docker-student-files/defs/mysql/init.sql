use iots_2025;

CREATE TABLE IF NOT EXISTS iot_data (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    address VARCHAR(100),
    location VARCHAR(100),
    device VARCHAR(100),
    metric_key ENUM('Temperature', 'Humidity', 'Airpressure') NOT NULL,
    value DOUBLE(10,3) NOT NULL,
    measure_time DATETIME DEFAULT CURRENT_TIMESTAMP
);