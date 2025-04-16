create database weather_iot;
create user iot_user with password 'aha987';
GRANT ALL PRIVILEGES ON DATABASE weather_iot TO iot_user;
use weather_iot;

CREATE TABLE sensor_data (
                             id SERIAL PRIMARY KEY,
                             time TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
                             humidity DECIMAL(5,2) CHECK (humidity >= 0 AND humidity <= 100),
                             temperature DECIMAL(5,2)
);

GRANT ALL PRIVILEGES ON TABLE sensor_data TO iot_user;

GRANT ALL PRIVILEGES ON SEQUENCE sensor_data_id_seq TO iot_user;

CREATE TABLE devices (
                         id SERIAL PRIMARY KEY,
                         name VARCHAR(100) NOT NULL UNIQUE,
                         password_hash bytea NOT NULL,
                         created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
                         last_seen TIMESTAMP WITH TIME ZONE
);

ALTER TABLE sensor_data
    ADD COLUMN device_id INTEGER NOT NULL,
    ADD CONSTRAINT fk_device
    FOREIGN KEY (device_id)
    REFERENCES devices(id)
    ON DELETE CASCADE;

GRANT ALL PRIVILEGES ON TABLE devices TO iot_user;
GRANT ALL PRIVILEGES ON SEQUENCE devices_id_seq TO iot_user;

CREATE TABLE IF NOT EXISTS tokens
(
    hash    bytea PRIMARY KEY,
    device_id bigint                      NOT NULL REFERENCES devices ON DELETE CASCADE,
    expiry  timestamp(0) with time zone NOT NULL,
    scope   text                        not null
);

GRANT ALL PRIVILEGES ON TABLE tokens TO iot_user;