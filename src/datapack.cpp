#include <stdio.h>
#include "pico/stdlib.h"

#include "datapack.h"

#define GPIO_GET_ON_STATE(pin, state) (!!(state & (1 << _pins[pin])))
#define GPIO_GET_PREV(pin) GPIO_GET_ON_STATE(pin, _prevRawState)
#define GPIO_GET(pin) GPIO_GET_ON_STATE(pin, _rawState)
#define GET_STATE_CHANGE(pin, edge) ((GPIO_GET(pin) != GPIO_GET_PREV(pin)) && (GPIO_GET(pin)) ^ edge)

Datapack::Datapack(DataBus pins) {
    _pins = pins;

    _data.fill(0);

    _data[0] = 1; // ID

    for (size_t i = 0; i < _pins.size(); i++) {
        gpio_init(_pins[i]);
    }
}

void Datapack::step() {
    readState();

    if (GPIO_GET(SS_B) == HIGH) {
        goto end;
    }

    if (GPIO_GET(SMR) == HIGH) {
        _mainCounter = 0;
        _pageCounter = 0;

        setDataValue(_data[0]);

        writeState();

        goto end;
    }

    if (GET_STATE_CHANGE(SCLK, EDGE_FALLING)) {
        _mainCounter += 2;
    }

    if (GET_STATE_CHANGE(SPGM_B, EDGE_FALLING)) {
        _pageCounter++;

        if (_pageCounter >= _data.size() >> 8) {
            _pageCounter = 0;
        }
    }

    if (GPIO_GET(SOE_B) == HIGH) {
        _data[getAddress()] = getDataValue();

        goto end;
    }

    setDataValue(_data[getAddress()]);

    writeState();

    end:

    tick++;
}

void Datapack::readState() {
    _prevState = _state;
    _input = sio_hw->gpio_in & (1 << _pins[SOE_B]);

    if (!_input) {
        // sio_hw->gpio_oe_set = 0b1111'1111'0000'0000'0000;
        for (size_t i = 0; i < 8; i++) {
            if (_pinDirections[i] != PinDirection::OUTPUT) {
                gpio_set_dir(_pins[i], GPIO_OUT);
            }

            _pinDirections[i] = PinDirection::OUTPUT;
        }
    } else {
        // sio_hw->gpio_oe_clr = 0b1111'1111'0000'0000'0000;
        // sio_hw->gpio_oe = 0;
        // gpio_init_mask(0b1111'1111'0000'0000'0000);
    }

    for (size_t i = _input ? 0 : 8; i < _pins.size(); i++) {
        if (_pinDirections[i] != PinDirection::INPUT) {
            gpio_init(_pins[i]);
            gpio_set_dir(_pins[i], GPIO_IN);
        }

        _pinDirections[i] = PinDirection::INPUT;

        // _state[i] = (_rawState & (1 << _pins[i])) ? 1 : 0;
        // _state[i] = gpio_get(_pins[i]);
    }

    // if (_input == _prevInput) {
    //     if (!_input) {
    //         sleep_us(2);
    //     }
    // } else {
    //     _prevInput = _input;
    // }

    _prevRawState = _rawState;
    _rawState = sio_hw->gpio_in;
}

void Datapack::writeState() {
    if (_input) {
        return;
    }

    sio_hw->gpio_out = _outputState;
    // sio_hw->gpio_clr = (~_outputState);
    // for (size_t i = 0; i < _pins.size(); i++) {
    //     // gpio_put(_pins[i], _state[i]);
        
    //     sio_hw->gpio_set = _state[i] ? (1 << _pins[i]) : 0;
    //     sio_hw->gpio_clr = _state[i] ? 0 : (1 << _pins[i]);
    // }
}

char Datapack::getDataValue() {
    char value = 0;

    for (size_t i = 0; i < 8; i++) {
        value |= GPIO_GET(i) << i;
    }

    return value;
}

void Datapack::setDataValue(char value) {
    _outputState = 0;

    for (size_t i = 0; i < 8; i++) {
        // _state[i] = (value & (1 << i)) ? 1 : 0;
        if (value & (1 << i)) {
            _outputState |= 1 << _pins[i];
        }
        // printf("value %02x, raw %x shift %d\n", value, _rawState >> 12, 12);
    }
}

size_t Datapack::getAddress() {
    return (_pageCounter << 8) | _mainCounter | GPIO_GET(SCLK);
}

// Edge Datapack::getStateChange(size_t index) {
//     if (_state[index] == _prevState[index]) {
//         return Edge::NONE;
//     }

//     return _state[index] ? Edge::RISING : Edge::FALLING;
// }

void Datapack::dumpMemory() {
    for (size_t i = 0; i < 256; i++) {
        size_t addr = (viewPage * 256) + i;

        if (i % 16 == 0) {
            printf("%04x| ", addr);
        }

        printf("%02x ", _data[addr]);

        if (i % 16 == 15) {
            printf("  ");

            for (size_t j = 0; j < 16; j++) {
                char c = _data[addr - 15 + j];

                if (c >= 32 && c <= 126) {
                    printf("%c", c);
                } else {
                    printf(".");
                }
            }

            printf("\n");
        }
    }

    printf("Current address: %04x (main counter %02x, page counter %02x)\n", getAddress(), _mainCounter, _pageCounter);
}