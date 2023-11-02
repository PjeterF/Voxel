#include "Utility.hpp"

namespace utility
{
    namespace pairing
    {
        int mapIntegerToNatural(int a)
        {
            if (a >= 0)
                return 2 * a;
            else
                return -2 * a - 1;
        }

        int cantorPair(int a, int b)
        {
            return (a + b) * (a + b + 1) / 2 + b;
        }

        int cantorTriple(int a, int b, int c)
        {
            return cantorPair(cantorPair(a, b), c);
        }

        int integerPair(int a, int b)
        {
            return cantorPair(mapIntegerToNatural(a), mapIntegerToNatural(b));
        }

        int integerTriple(int a, int b, int c)
        {
            return integerPair(integerPair(a, b), c);
        }
    }
}