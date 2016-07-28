#include "RandomNumberGenerator.h"
#include <ctime>
#include <cstdlib>
#include <random>

#if RAND_MAX != 0x7FFFFFFF
#error unexpected RAND_MAX value
#endif

#if CLOCKS_PER_SEC != 1000000
#error unexpected CLOCKS_PER_SEC value
#endif

namespace produce_questions
{
    bool RandomNumberGenerator::_initialized = false;
    unsigned int RandomNumberGenerator::_shift[32];
    int RandomNumberGenerator::_counter = 0;

    void RandomNumberGenerator::initialize(void)
    {
        srand(static_cast<unsigned int>(time(0)));

        std::random_device rd;
        unsigned int n = rd();
        int i;
        for (i = 0; i < 32; ++i)
        {
            _shift[i] = n;
            n = (n << 1) | (n >> (32 - 1));
        }

        _initialized = true;
    }

    int RandomNumberGenerator::verify(void)
    {
        if (!_initialized) initialize();
        if (std::random_device::min() != 0 || std::random_device::max() != 0xFFFFFFFF) return -1;
        return 0;
    }

    unsigned int RandomNumberGenerator::shift(void)
    {
        ++_counter;
        if (_counter == 32) _counter = 0;
        return _shift[_counter];
    }

    unsigned int RandomNumberGenerator::getRandomInt(void)
    {
        if (!_initialized) initialize();
        return static_cast<unsigned int>(rand()) + shift() + static_cast<unsigned int>(std::clock());
    }

    unsigned int RandomNumberGenerator::getRandomInt(int valueCount)
    {
        return getRandomInt() % valueCount;
    }
}
