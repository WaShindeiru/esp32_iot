package data

import (
	"database/sql"
	"time"
)

type SensorData struct {
	Id          int64     `json:"id"`
	Time        time.Time `json:"time"`
	Humidity    float64   `json:"humidity"`
	Temperature float64   `json:"temperature"`
	Device_id   int64     `json:"device id"`
}

type SensorDataRepository struct {
	Db *sql.DB
}

func (d SensorDataRepository) Insert(sensor_data *SensorData) (*SensorData, error) {
	query := `
	insert into sensor_data (time, humidity, temperature, device_id) values ($1, $2, $3, $4) returning id;
	`
	err := d.Db.QueryRow(query, sensor_data.Time, sensor_data.Humidity, sensor_data.Temperature, sensor_data.Device_id).
		Scan(&sensor_data.Id)
	if err != nil {
		return nil, err
	}

	return sensor_data, nil
}

func (d SensorDataRepository) GetAllForDevice(device_id int64) ([]SensorData, error) {
	query := `
	select id, time, humidity, temperature from sensor_data where device_id = $1;
	`
	rows, err := d.Db.Query(query, device_id)
	if err != nil {
		return nil, err
	}

	defer rows.Close()

	sensor_data := make([]SensorData, 0)
	for rows.Next() {
		sensor_data_row := SensorData{}
		err = rows.Scan(&sensor_data_row.Id, &sensor_data_row.Time, &sensor_data_row.Humidity, &sensor_data_row.Temperature)
		if err != nil {
			return nil, err
		}
		sensor_data = append(sensor_data, sensor_data_row)
	}

	return sensor_data, nil
}

//func (d SensorDataRepository) RemoveByName(name string) error {
//	query := `
//	delete from sensor_data where name = $1
//	`
//	_, err := d.Db.Exec(query, name)
//	return err
//}
