#include "WProgram.h"

int led_state = LOW;


void led_on(void) {
    digitalWriteFast(13, HIGH);
}


void led_off(void) {
    digitalWriteFast(13, LOW);
}


void blink(int count, int ms_delay) {
    led_off();
    delay(ms_delay);
    while (count > 0) {
        led_on();
        delay(ms_delay);
        led_off();
        delay(ms_delay);
        count --;
    }
    digitalWriteFast(13, led_state);
}


void usb_send_packet(int addr, uint16_t data) {
    uint8_t packet[3];

    // remember what we've sent, and dont sent it again
    static bool have_sent_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static uint16_t sent_data[8];

    if ((have_sent_data[addr]) && (sent_data[addr] == data)) {
        return;
    }

    have_sent_data[addr] = 1;
    sent_data[addr] = data;

    packet[0] = 0x00;
    packet[0] |= (addr & 0x07) << 4;
    packet[0] |= (data & 0xf000) >> 12;

    packet[1] = 0x80;
    packet[1] |= (data & 0x0fc0) >> 6;

    packet[2] = 0xc0;
    packet[2] |= data & 0x003f;

    usb_serial_write(packet, 3);
    usb_serial_flush_output();
}


void usb_send_digital_inputs(void) {
    uint8_t addr;   // 3 bits used
    uint16_t data;  // all 16 bits used

    addr = 0;
    data = 0;
    for (int i = 0; i < 7; i ++) {
        if (digitalRead(i)) {
            data |= 1<<i;
        }
    }
    usb_send_packet(addr, data);
}


void usb_send_analog_inputs(void) {
    uint8_t addr;
    uint16_t data;

    addr = 1;
    data = analogRead(14);
    usb_send_packet(addr, data);

    addr = 2;
    data = analogRead(15);
    usb_send_packet(addr, data);
}


void handle_digital_outputs(uint16_t data) {
    for (int i = 0; i < 7; i ++) {
        if (data & (1<<i)) {
            digitalWriteFast(7+i, HIGH);
        } else {
            digitalWriteFast(7+i, LOW);
        }
    }
    if (data & (1 << 6)) {
        led_state = HIGH;
    } else {
        led_state = LOW;
    }
}


// data is 16 bits, but the actual analog output is only 12
void handle_analog_output_0(uint16_t data) {
    data >>= 4;
    analogWriteDAC0(data);
}


void handle_pwm_channel(int channel, uint16_t data) {
    analogWrite(20 + channel, data);
}


void handle_incoming_packet(uint8_t buf[3]) {
    uint8_t addr;
    uint16_t data;

    addr = (buf[0] >> 4) & 0x07;
    data = ((uint16_t)(buf[0] & 0x0f)) << 12;
    data |= (buf[1] & 0x3f) << 6;
    data |= (buf[2] & 0x3f);

    switch (addr) {
        case 0:
            // digital outputs
            handle_digital_outputs(data);
            break;

        case 1:
            // analog output
            handle_analog_output_0(data);
            break;

        case 2:
        case 3:
        case 4:
        case 5:
            handle_pwm_channel(addr-2, data);
            break;

        default:
            // unhandled packet!
            blink(4, 100);
            break;
    }
}


void read_usb(void) {
    // the buffer is valid up to but not including index
    static int index = 0;
    static uint8_t buf[3];

    int new_index;
    uint8_t new_bytes[3];

    int bytes_avail;
    int bytes_wanted;
    int bytes_read;

    bytes_wanted = 3 - index;

    bytes_avail = usb_serial_available();
    if (bytes_avail == 0) {
        return;
    }

    if (bytes_avail < bytes_wanted) {
        bytes_wanted = bytes_avail;
    }

    bytes_read = usb_serial_read(new_bytes, bytes_wanted);
    new_index = 0;

    while (new_index < bytes_read) {
        switch (index) {
            case 0:
                if ((new_bytes[new_index] & 0x80) == 0x00) {
                    buf[index] = new_bytes[new_index];
                    index ++;
                    new_index ++;
                } else {
                    // invalid byte in stream, discard the packet we were assembling
                    index = 0;
                    new_index ++;
                }
                break;

            case 1:
                if ((new_bytes[new_index] & 0xc0) == 0x80) {
                    buf[index] = new_bytes[new_index];
                    index ++;
                    new_index ++;
                } else {
                    // invalid byte in stream, discard the packet we were assembling
                    index = 0;
                }
                break;

            case 2:
                if ((new_bytes[new_index] & 0xc0) == 0xc0) {
                    buf[index] = new_bytes[new_index];
                    // blink(2, 150);
                    handle_incoming_packet(buf);
                    index = 0;
                    new_index ++;
                } else {
                    // invalid byte in stream, discard the packet we were assembling
                    index = 0;
                }
                break;

            default:
                // internal error
                // blink(5, 250);
                index = 0;
                break;
        }
    }
}


extern "C" int main(void) {
    analogReadRes(16);

    // digital inputs
    pinMode(0, INPUT);
    pinMode(1, INPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);

    // digital outputs
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);

    // LED (acts like a digital output)
    pinMode(13, OUTPUT);

    blink(2, 50);

    while (1) {
        usb_send_digital_inputs();
        usb_send_analog_inputs();
        read_usb();
        delay(10);
    }
}
