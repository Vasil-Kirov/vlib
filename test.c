#include "vlib.h"

#pragma pack(push, 1)
typedef struct
{
    int y;
    short x;
} some_data;
#pragma pack(pop)

int main()
{
    InitVLib();
    printf("size of struct %zd\n", sizeof(some_data));
    some_data *MyArr = ArrCreate(some_data);
    for (int i = 0; i < 50; ++i)
    {
        some_data It = {.x = i * 2, .y = i};
        ArrPush(MyArr, It);
    }
    for(int i = 0; i < 50; ++i)
        printf("It X: %d It Y: %d\n", MyArr[i].x, MyArr[i].y);

    while(true)
    {
        printf("%llu\n", ClockNs());
    }
}