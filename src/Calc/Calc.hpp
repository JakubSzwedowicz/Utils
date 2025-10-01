#include "inc/utils.hpp"

namespace Calc
{
    class Calc
    {
        public:
            static int add(int a, int b);

            template <typename T>
            static T templateAdd(T a, T b);
    };

    template <typename T>
    T Calc::templateAdd(T a, T b)
    {

        return a + b;
    }
}