#ifndef DATAPACK_H_
#define DATAPACK_H_

#include <array>

#define SMR 8
#define SCLK 9
#define SOE_B 10
#define SS_B 11
#define SPGM_B 12

#define HIGH 1
#define LOW 0

typedef int Pin;
typedef std::array<Pin, 13> DataBus;

enum Edge {
    NONE = 0,
    RISING = 1,
    FALLING = -1
};

enum PinDirection {
    UNSET = 0,
    INPUT = 1,
    OUTPUT = 2
};

class Datapack {
    public:
        Datapack(DataBus pins);

        void step();
        void readState();
        void writeState();
        char getDataValue();
        void setDataValue(char value);
        size_t getAddress();
        Edge getStateChange(size_t index);
        void dumpMemory();

        size_t tick = 0;
        size_t flips = 0;

    private:
        DataBus _pins;
        DataBus _pinDirections;
        DataBus _state;
        DataBus _prevState;
        size_t _checksum = 0;
        size_t _prevChecksum = 0;

        bool _input = false;
        uint64_t _lastWrite = 0;
        bool _forceWrite = false;
        char _lastValue = 0;
        uint32_t _pinState = 0;

        std::array<char, 32 * 1024> _data;
        unsigned char _mainCounter = 0;
        unsigned char _pageCounter = 0;
};

#endif