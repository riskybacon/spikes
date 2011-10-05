#include <iostream>

template <bool B, class T = void>
struct enable_if_c {
   typedef T type;
};

template <class T>
struct enable_if_c<false, T> {};


/**
 *
 */
template<bool CONDITION, class T = void>
struct EnableIf
{
   typedef T type;
};

template<class T>
struct EnableIf<false, T> {};


template <long x, typename enabled=void>
struct Abs 
{
   const static long value_static = x;
   enum { result = x } ;
};

template <long x>
struct Abs<x,typename EnableIf<(x < 0)>::type>
{
   const static long value_static = -x;
   enum { result = -x } ;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Calculate the absolute value of an integer
/////////////////////////////////////////////////////////////////////////////////////////////

#if 0
template<int N, typename enabled = void>
class Abs_new
{
public:
   enum { result = N } ;
};

template<int N>
class Abs_new
{
};
#endif

#if 0
template<long N>
class Abs
{
public:
   // Compile time state
   static int const result = (N >= 0) ? N : -N;
};
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
/// Calculate the greatest common divisor of two integers
/////////////////////////////////////////////////////////////////////////////////////////////
template<int M, int N>
class Gcd
{
private:
   // Compile time state
   enum { m = Abs<M>::result };
   enum { n = Abs<N>::result };
   
public:
   // Compile time state
   enum { result = Gcd<n, m % n>::result } ;
};

template<int M>
class Gcd<M, 0>
{
public:
   // Compile time state
#if 0
   enum { result = (M == 0) ? 1 : Abs<M>::result } ;
#else
   static const long result = (M == 0) ? 1 : Abs<M>::result;
#endif
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Represent two integers as a ratio. They are divided by their greatest common divisor
/// so that the smallest possible integers are stored
/////////////////////////////////////////////////////////////////////////////////////////////
template<long M, long N>
class Ratio
{
public:
   // Compile time state
#if 1
   static int const gcd = Gcd<M,N>::result;
   static int const m = M / gcd;
   static int const n = N / gcd;
#else
   enum { gcd = Gcd<M,N>::result };
   static int const m = M / gcd;
   static int const n = N / gcd;
#endif
   typedef Ratio<M,N> type;
   
   // Display a ratio
   template<int M1, int N1> friend std::ostream& operator<<(std::ostream&, const Ratio<M1,N1>&);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Multiply two ratios
/////////////////////////////////////////////////////////////////////////////////////////////
template<class A, class B>
struct MultRatio
{
};

template<int M0, int M1, int N0, int N1, template<int, int> class Ratio >
class MultRatio<Ratio<M0, N0>, Ratio<M1, N1> >
{
public:
   // Compile time state
   typedef Ratio<M0 * M1, N0 * N1> result;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Divide two ratios
/////////////////////////////////////////////////////////////////////////////////////////////
template<class A, class B>
struct DivRatio
{
};

template<int M0, int M1, int N0, int N1, template<int, int> class Ratio >
class DivRatio<Ratio<M0, N0>, Ratio<M1, N1> >
{
public:
   // Compile time state
   typedef Ratio<M0 * N1, N0 * M1> result;
};


/////////////////////////////////////////////////////////////////////////////////////////////
/// Add two ratios
/////////////////////////////////////////////////////////////////////////////////////////////
template<class A, class B>
struct AddRatio
{
};

template<int M0, int M1, int N0, int N1, template<int, int> class Ratio >
class AddRatio<Ratio<M0, N0>, Ratio<M1, N1> >
{
public:
   // Compile time state
   typedef Ratio<M0 * N1 + M1 * N0, N0 * N1> result;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Subtract two ratios
/////////////////////////////////////////////////////////////////////////////////////////////
template<class A, class B>
struct SubRatio
{
};

template<int M0, int M1, int N0, int N1, template<int, int> class Ratio >
class SubRatio<Ratio<M0, N0>, Ratio<M1, N1> >
{
public:
   // Compile time state
   typedef Ratio<M0 * N1 - M1 * N0, N0 * N1> result;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// Print out a ratio
/////////////////////////////////////////////////////////////////////////////////////////////
template<int M, int N>
std::ostream&
operator<<(std::ostream &os, const Ratio<M,N>& ratio)
{
   os << ratio.m << "/" << ratio.n;
   return os;
}

template<int N_RET, int V_RET, int M1, int N1, int M2, int N2>
typename Ratio<N_RET, V_RET>::result
multiply(const Ratio<M1, N1>& r1, const Ratio<M2, N2>& r2)
{
   return MultRatio< Ratio<M1,N1>, Ratio<M2,N2> >::result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// Entry point
/////////////////////////////////////////////////////////////////////////////////////////////
int main(int, char**)
{
   std::cout << "gcd of 10 and 10: " << Gcd<10, 5>::result << std::endl;
   
   Ratio<15000,30> r1;
   Ratio<39*30,39> r2;
   Ratio<30,15000> r3;
   Ratio<1500,3>   r4; 

      // The next two types are the same. Under XCode 3.2.5 and gcc 4.5, this
   // compiles, but the Boost folks seem to believe that this will fail and
   // instead this syntax should be used:
   //
   // typedef Ratio<15000,30>::type ratio500_1a;
   //
   typedef Ratio<15000,30> ratio500_1a;
   typedef Ratio<1500,3>   ratio500_1b;

   std::cout << r1 << std::endl;
   std::cout << r2 << std::endl;
   std::cout << r3 << std::endl;
   
   MultRatio< Ratio<1500,30>, Ratio<3,2> >::result mr1;
//   Ratio<> mr2 = multiply<r1::type, r2::type>(r1, r2);

   AddRatio<  Ratio<15000,30>, Ratio<326,3> >::result mr2;
   SubRatio<  Ratio<2,3>,      Ratio<1,3> >::result   sr1;
   SubRatio<  Ratio<9,10>,     Ratio<1,100> >::result sr2;
   AddRatio<  Ratio<9,10>,     Ratio<1,100> >::result ar2;
   
   std::cout << "multratio: " << mr1 << std::endl;
   std::cout << "addratio: " << mr2  << std::endl;
   std::cout << "subratio: " << sr1  << std::endl;
   std::cout << "subratio: " << sr2  << std::endl;
   std::cout << "addratio: " << ar2  << std::endl;
   std::cout << "r4:       " << r4   << std::endl;
   return 0;
}
