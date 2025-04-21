#ifndef DHT_11
#define DHT_11

typedef struct
{
    int dht11_pin;
    float temperature;
    float humidity;
} dht11_t;

int wait_for_state(dht11_t dht11,int state,int timeout);

void hold_low(dht11_t dht11,int hold_time_us);

void hold_low_ms(dht11_t dht11,int hold_time_ms);

void set_input(void);

void set_output(void);

int dht11_read(dht11_t *dht11,int connection_timeout);

#endif