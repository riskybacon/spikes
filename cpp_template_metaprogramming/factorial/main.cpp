#include <iostream>

/**
 * Define the factorial class.
 */
template<int N>
struct factorial
{
   static const int result = N * factorial<N - 1>::result;
};

/**
 * Factorial base case
 */
template<>
struct factorial<0>
{
   static const int result = 1;
};


/**
 * Program entry point
 */
int main(int, char**)
{
   std::cout << " 1! = " << factorial< 1>::result << std::endl;
   std::cout << " 2! = " << factorial< 2>::result << std::endl;
   std::cout << " 3! = " << factorial< 3>::result << std::endl;
   std::cout << "10! = " << factorial<10>::result << std::endl;
   std::cout << "20! = " << factorial<20>::result << std::endl; // Overflow!

   return 0;
}
