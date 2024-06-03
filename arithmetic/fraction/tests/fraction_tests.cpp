#include <fraction.h>
#include <iostream>
#include <cmath>

using std::cout;
using std::cin;
using std::endl;


// fraction calc_pi(fraction eps) {
//     fraction x0 = fraction(big_integer(3), big_integer(1));
//     fraction x1 = x0;
//     do {
//         x0 = x1;
//         x1 = x0 + (x0.cos(eps) + fraction(big_integer(1), big_integer(1)) / x0.sin(eps)); 
//     } while ((x0 - x1).abs() > eps);
//     return x1;
// }


int main(int argc,char *argv[])
{
    // big_integer num(-5);
    // big_integer num2("-5");

    // num.dump_value(cout);
    // cout << endl;
    // num2.dump_value(cout);
    // cout << endl;

    // cout << (num) << endl;
	


    fraction a(big_integer(1), big_integer(3));
    // fraction b(big_integer(-0), big_integer(15));

    fraction eps(big_integer(1), big_integer("1000000"));
    

    // fraction c = a.cos(eps);
    cout << a.arcsin(eps) << endl;
    

    return 0;
}