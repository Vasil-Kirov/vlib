#include "vlib.h"
#include <vector>

#pragma pack(push, 1)
typedef struct
{
    int y;
    short x;
} some_data;
#pragma pack(pop)

#define ITs 5000000

int main()
{
    InitVLib();
    printf("size of struct %zd\n", sizeof(some_data));

    timer_group VLibArr = VLibStartTimer("VLib Dynamic Array");

    some_data *MyArr = ArrCreate(some_data);
    for (int i = 0; i < ITs; ++i)
    {
        some_data It = {i * 2, (short)i};
        VLibArrPush(MyArr, It);
    }
    ArrFree(MyArr);
    VLibStopTimer(&VLibArr);

    timer_group VecTimer = VLibStartTimer("Std Vector Array");
    std::vector<some_data> vec;
    for (int i = 0; i < ITs; ++i)
    {
        some_data It = {i * 2, (short)i};
        vec.push_back(It);
    }
    VLibStopTimer(&VecTimer);

    VLibCompareTimers(VLibArr, VecTimer);
}