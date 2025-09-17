#include <stdio.h>
#include "pico/stdlib.h"

#include "datapack.h"

Datapack::Datapack(DataBus pins) {
    _pins = pins;

    _data.fill(0xFF);

    // _data[0] = 1; // ID

    std::array<char, 48> initialData = {
        0x7c, 0x04, 0x29, 0x06, 0x0f, 0x0b, 0x58, 0xd3, 0x0c, 0xe8, 0x09, 0x81, 0x4d, 0x41, 0x49, 0x4e,
        0x20, 0x20, 0x20, 0x20, 0x90, 0x18, 0x90, 0x54, 0x45, 0x53, 0x54, 0x49, 0x4e, 0x47, 0x09, 0x48,
        0x45, 0x4c, 0x4c, 0x4f, 0x20, 0x57, 0x4f, 0x52, 0x4c, 0x44, 0x20, 0x31, 0x32, 0x33, 0x34, 0xff
        // 0x6a, 0x01, 0x00, 0x41, 0x10, 0x41, 0x00, 0x19, 0xff, 0xff, 0x09, 0x81,
        // 0x4d, 0x41, 0x49, 0x4e, 0x20, 0x20, 0x20, 0x20, 0x90, 0x02, 0x80, 0x00,
        // 0xa5, 0x00, 0x85, 0x00, 0x00, 0x00, 0x41, 0x10, 0x02, 0x00, 0x12, 0x00,
        // 0x45, 0x05, 0x48, 0x45, 0x4c, 0x4c, 0x4f, 0x00, 0x68, 0x86, 0x0c, 0x3f,
        // 0x10, 0xf6, 0x00, 0x36, 0xce, 0x00, 0x37, 0x3f, 0x11, 0x4f, 0xf6, 0x00,
        // 0x0a, 0xcb, 0x03, 0xdd, 0x41, 0xce, 0x00, 0x0a, 0xcc, 0x21, 0x87, 0x3f,
        // 0x6d, 0xc6, 0x00, 0x3f, 0x65, 0x3f, 0x48, 0x0c, 0x39, 0x0e, 0x49, 0x6e,
        // 0x73, 0x74, 0x61, 0x6c, 0x6c, 0x20, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72,
        // 0x86, 0x0c, 0x3f, 0x10, 0xf6, 0x00, 0x5a, 0xce, 0x00, 0x5b, 0x3f, 0x11,
        // 0xce, 0x00, 0x0a, 0x3f, 0x67, 0x3f, 0x48, 0x0c, 0x39, 0x0d, 0x52, 0x65,
        // 0x6d, 0x6f, 0x76, 0x65, 0x20, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x86,
        // 0x0c, 0x3f, 0x10, 0xf6, 0x00, 0x77, 0xce, 0x00, 0x78, 0x3f, 0x11, 0x3f,
        // 0x48, 0x39, 0x0d, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77, 0x6f,
        // 0x72, 0x6c, 0x64, 0x21, 0x28, 0x4a, 0x00, 0x0c, 0x00, 0x06, 0x00, 0x08,
        // 0x00, 0x10, 0x00, 0x17, 0x00, 0x1a, 0x00, 0x20, 0x00, 0x27, 0x00, 0x4a,
        // 0x00, 0x4d, 0x00, 0x52, 0x00, 0x6d, 0x00, 0x70, 0x02, 0x5c
    };

    std::copy(std::begin(initialData), std::end(initialData), std::begin(_data));

    for (unsigned int i = 0; i < _pins.size(); i++) {
        gpio_init(_pins[i]);
    }
}

void Datapack::step() {
    readState();

    if (_state[SS_B] == HIGH) {
        return;
    }

    if (_state[SOE_B] == LOW) {
        // printf("Data: %08x %02x\n", getAddress(), _data[getAddress()]);
        setDataValue(_data[getAddress()]);

        // writeState();
    }

    if (_state[SMR] == HIGH) {
        _mainCounter = 0;
        _pageCounter = 0;

        // setDataValue(1);

        // writeState();

        return;
    }

    if (getStateChange(SCLK) == Edge::FALLING) {
        _mainCounter += 2;
    }

    if (getStateChange(SPGM_B) == Edge::FALLING) {
        _pageCounter++;

        if (_pageCounter >= _data.size() >> 8) {
            _pageCounter = 0;
        }
    }

    if (_state[SOE_B] == HIGH) {
        _data[getAddress()] = getDataValue();

        return;
    }
}

void Datapack::readState() {
    _prevState = _state;
    _input = !!(sio_hw->gpio_in & (1 << _pins[SOE_B]));

    if (!_input) {
        for (size_t i = 0; i < 8; i++) {
            if (_pinDirections[i] != PinDirection::OUTPUT) {
                gpio_set_dir(_pins[i], GPIO_OUT);
            }

            _pinDirections[i] = PinDirection::OUTPUT;
        }

        _forceWrite = true;
    }

    // _checksum = 0;

    size_t passes = 0;

    // while (passes < 2) {
    // size_t currentChecksum = 0;

    for (size_t i = _input ? 0 : 8; i < _pins.size(); i++) {
        if (_pinDirections[i] != PinDirection::INPUT) {
            gpio_init(_pins[i]);
            gpio_set_dir(_pins[i], GPIO_IN);
        }

        _pinDirections[i] = PinDirection::INPUT;

        // _state[i] = gpio_get(_pins[i]);
        _state[i] = !!(sio_hw->gpio_in & (1 << _pins[i]));
        // currentChecksum += _state[i];
    }

        // if (currentChecksum == _checksum) {
        //     passes++;
        // } else {
        //     _checksum = currentChecksum;
        //     passes = 0;
        // }
    // }

    // if (_checksum != _prevChecksum) {
    //     flips++;
    // }

    tick++;
    // _prevChecksum = _checksum;
}

void Datapack::writeState() {
    if (_input) {
        return;
    }

    uint32_t value = 0;

    // sio_hw->gpio_out = 0;
    // sio_hw->gpio_oe_set = 0b1111'1111'0000'0000'0000;

    // sleep_us(10);

    for (size_t i = 0; i < _pins.size(); i++) {
        gpio_put(_pins[i], _state[i]);

        // if (!_state[i]) {
        //     continue;
        // }

        // value |= (1 << _pins[i]);
    }

    // gpio_put_all(value);

    // sio_hw->gpio_clr = ~(value);
    // sio_hw->gpio_set = value;
    // printf("set %x\n", sio_hw->gpio_out);
}

char Datapack::getDataValue() {
    char value = 0;

    for (size_t i = 0; i < 8; i++) {
        value |= _state[i] << i;
    }

    return value;
}

void Datapack::setDataValue(char value) {
    if (!_forceWrite && value == _lastValue) {
    //     // writeState();
        // gpio_put_all(_pinState);
        return;
    }

    _forceWrite = false;
    _lastValue = value;

    _pinState = 0;

    for (size_t i = 0; i < 8; i++) {
        // _state[i] = (value & (1 << i)) ? 1 : 0;

        gpio_put(_pins[i], value & (1 << i));

        // if (!(value & (1 << i))) {
        //     continue;
        // }

        // _pinState |= (1 << _pins[i]);
    }

    // for (size_t i = 0; i < _pins.size(); i++) {
    //     // gpio_put(_pins[i], _state[i]);
    //     if (!_state[i]) {
    //         continue;
    //     }

    //     value |= (1 << _pins[i]);
    // }

    // gpio_put_all(value);

    // sio_hw->gpio_clr = ~(_pinState);
    // sio_hw->gpio_out = _pinState;

    // gpio_put_all(_pinState);

    // writeState();
}

size_t Datapack::getAddress() {
    return (_pageCounter << 8) | _mainCounter | _state[SCLK];
}

Edge Datapack::getStateChange(size_t index) {
    if (_state[index] == _prevState[index]) {
        return Edge::NONE;
    }

    return _state[index] ? Edge::RISING : Edge::FALLING;
}

void Datapack::dumpMemory() {
    for (size_t i = 0; i < 256; i++) {
        if (i % 16 == 0) {
            printf("%02x| ", i);
        }

        printf("%02x ", _data[i]);

        if (i % 16 == 15) {
            printf("  ");

            for (size_t j = 0; j < 16; j++) {
                char c = _data[i - 15 + j];

                if (c >= 32 && c <= 126) {
                    printf("%c", c);
                } else {
                    printf(".");
                }
            }

            printf("\n");
        }
    }

    printf("Current address: %x (main counter %x, page counter %x)\n", getAddress(), _mainCounter, _pageCounter);
}