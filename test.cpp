#define _CRT_SECURE_NO_WARNINGS
#define VLIB_IMPLEMENTATION
#include "vlib.h"
//#include <vector>
#include <iostream>
using namespace std;


struct some_data
{
    int y;
    short x;
} __attribute__ ((packed));


#define ITs 5000000

int main()
{
#if 0
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
    vector<some_data> vec;
    for (int i = 0; i < ITs; ++i)
    {
        some_data It = {i * 2, (short)i};
        vec.push_back(It);
    }
    VLibStopTimer(&VecTimer);

    VLibCompareTimers(VLibArr, VecTimer);
#else
	TokinizeAddKeyword("foo");
	token *Tokens = VLibTokinizeString("foo { string = \"-1.0.y\" }");
	for(int i = 0; i < ArrLen(Tokens); ++i)
	{
		cout << Tokens[i].Type << " ";
		cout.write(Tokens[i].ID.Str, Tokens[i].ID.Len);
		cout << endl;
	}
	ArrFree(Tokens);
#endif
}
