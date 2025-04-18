package data

import (
	"server/internal/util"
	"testing"
	"time"
)

func TestInsertGet(t *testing.T) {
	device := &Device{
		Name:      "eps32",
		CreatedAt: time.Now(),
		LastSeen:  time.Now(),
	}

	err := device.Password.Set("monke")
	if err != nil {
		t.Fatal("can't set password")
	}

	db, err := OpenDB()
	if err != nil {
		t.Fatal(err)
	}

	repository := DeviceRepository{db}
	var device_result *Device
	device_result, err = repository.Insert(device)
	if err != nil {
		t.Fatal(err)
	}

	data_1 := &SensorData{
		Time:        time.Now(),
		Humidity:    10,
		Temperature: 20,
		Device_id:   device_result.Id,
	}

	data_2 := &SensorData{
		Time:        time.Now(),
		Humidity:    20,
		Temperature: 30,
		Device_id:   device_result.Id,
	}

	data_slice := []*SensorData{data_1, data_2}

	data_repository := SensorDataRepository{db}
	_, err = data_repository.Insert(data_1)
	util.AssertErrorNil(t, err)

	_, err = data_repository.Insert(data_2)
	util.AssertErrorNil(t, err)

	data_all, err := data_repository.GetAllForDevice(data_1.Device_id)
	util.AssertErrorNil(t, err)

	util.AssertEqual(t, 2, len(data_all))

	for i, _ := range data_all {
		util.AssertEqual(t, data_slice[i].Humidity, data_all[i].Humidity)
		util.AssertEqual(t, data_slice[i].Temperature, data_all[i].Temperature)
		util.Assert(t, data_slice[i].Time.Truncate(time.Second).Equal(data_all[i].Time.Truncate(time.Second)),
			"wrong timestamp")
	}
}
