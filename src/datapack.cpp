#include <stdio.h>
#include "pico/stdlib.h"

#include "datapack.h"

Datapack::Datapack(DataBus pins) {
    _pins = pins;

    _data.fill(0);

    _data[0] = 1; // ID
}

void Datapack::step() {
    readState();

    if (_state[SS_B] == HIGH) {
        goto end;
    }

    if (_state[SMR] == HIGH) {
        _mainCounter = 0;
        _pageCounter = 0;

        setDataValue(_data[0]);

        writeState();

        goto end;
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

        goto end;
    }

    setDataValue(_data[getAddress()]);

    writeState();

    end:

    tick++;
}

void Datapack::readState() {
    _prevState = _state;
    _input = gpio_get(_pins[SOE_B]);

    if (!_input) {
        for (size_t i = 0; i < 8; i++) {
            if (_pinDirections[i] != PinDirection::OUTPUT) {
                gpio_set_dir(_pins[i], GPIO_OUT);
            }

            _pinDirections[i] = PinDirection::OUTPUT;
        }
    }

    for (size_t i = _input ? 0 : 8; i < _pins.size(); i++) {
        if (_pinDirections[i] != PinDirection::INPUT) {
            gpio_init(_pins[i]);
            gpio_set_dir(_pins[i], GPIO_IN);
        }

        _pinDirections[i] = PinDirection::INPUT;

        _state[i] = gpio_get(_pins[i]);
    }
}

void Datapack::writeState() {
    if (_input) {
        return;
    }

    for (size_t i = 0; i < _pins.size(); i++) {
        gpio_put(_pins[i], _state[i]);
    }
}

char Datapack::getDataValue() {
    char value = 0;

    for (size_t i = 0; i < 8; i++) {
        value |= _state[i] << i;
    }

    return value;
}

void Datapack::setDataValue(char value) {
    for (size_t i = 0; i < 8; i++) {
        _state[i] = (value & (1 << i)) ? 1 : 0;
    }
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