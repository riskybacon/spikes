/////////////////////////////////////////////////////////////////////////////////
// This program demonstrates how to get and use pointers to functions and
// class methods.
//
// The following methods for getting pointers is demonstrated:
//
// function without arguments, no return value
// function with two arguments, no return value
// function with two arguments and a return value
// templatized function without arguments, no return value
// templatized function with two arguments, no return value
// templatized function with two arguments and templatized return value
// class method without arguments, no return value
// class method with two arguments, no return value
// class method with two arguments and a return value
// templatized class, non-templatized method without arguments, no return value
// templatized class, non-templatized method with two arguments, no return value
// templatized class, non-templatized method with two arguments and templatized return value
// templatized class, templatized method without arguments, no return value
// templatized class, templatized method with two arguments, no return value
// templatized class, templatized method with two arguments and templatized return value
//
// This is by no means an exhaustive set of combinations of templates, class
// arguments and return values. It is hoped that these combinations are enough
// to guide one through the process of getting whichever combination is needed.
//
// One obvious combination that is missing:
//
// Non-templatized class, templatized methods
/////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <assert.h>

// Keep track of the number of functions called so that
// a simple test to verify that all functions were called
// can be performed later.
int numFunctionsCalled = 0;

/////////////////////////////////////////////////////////////////////////////////
// Functions, no class involved
/////////////////////////////////////////////////////////////////////////////////

// Function with no return value, no arguments
void func0(void)
{
   std::cout << "func0() called" << std::endl;
   numFunctionsCalled++;
}

// Function with no return value, two arguments
void func1(int a, float b)
{
   std::cout << "func1(" << a << "," << b << ") called" << std::endl;
   numFunctionsCalled++;
}

// Function with a return value, two arguments
float func2(int a, float b)
{
   std::cout << "func2(" << a << "," << b << ") called" << std::endl;
   numFunctionsCalled++;
   return 6.67e-11f;
}

// Function with const arguments
void func3(const int a)
{
   std::cout << "func3(const int " << a << ") called" << std::endl;
   numFunctionsCalled++;
}

// Template function with no return value, no arguments.
template<typename T>
void tfunc0(void)
{
   std::cout << "tfunc0() called" << std::endl;
   numFunctionsCalled++;
}

// Template function with no return value, two arguments
template<typename A, typename B>
void tfunc1(A a, B b)
{
   std::cout << "tfunc1(" << a << "," << b << ") called" << std::endl;
   numFunctionsCalled++;
}

// Template function with a return value, two arguments
template<typename C, typename A, typename B>
C tfunc2(A a, B b)
{
   std::cout << "tfunc2(" << a << "," << b << ") called" << std::endl;
   numFunctionsCalled++;
   // C had better be a number type, or else this will fail to compile, but
   // this is just a demonstration and isn't meant to be robust
   return static_cast<C>(6.67e-11f);
}

////////////////////////////////////////////////////////////////////////////////
// Classes with methods
////////////////////////////////////////////////////////////////////////////////

// Class with some methods
class TestClass
{
public:
   // Method with no return value, no arguments
   void func0(void)
   {
      std::cout << "TestClass::func0() called" << std::endl;
      numFunctionsCalled++;
   }
   
   // Method with no return value, two arguments
   void func1(int a, float b)
   {
      std::cout << "TestClass::func1(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
   }
   
   // Method with a return value, two arguments
   float func2(int a, float b)
   {
      std::cout << "TestClass::func2(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
      return 6.67e-11f;
   }
};

// Templated class with some methods
template<typename T0>
class TemplateClass
{
public:
   /// Method with no return value, no arguments
   void func0(void)
   {
      std::cout << "TemplateClass<T>::func0() called" << std::endl;
      numFunctionsCalled++;
   }
   
   /// Method with no return value, two arguments
   void func1(int a, float b)
   {
      std::cout << "TemplateClass<T>::func1(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
   }
   
   /// Method with a return value, two arguments
   float func2(int a, float b)
   {
      std::cout << "TemplateClass<T>::func2(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
      return 6.67e-11f;
   }

   /// Method with no return value, no arguments
   template<typename A>
   void tfunc0(void)
   {
      std::cout << "TemplateClass<T>::tfunc0<A>() called" << std::endl;
      numFunctionsCalled++;
   }
   
   /// Method with no return value, two arguments
   template<typename A, typename B>
   void tfunc1(A a, B b)
   {
      std::cout << "TemplateClass<T>::tfunc1<A,B>(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
   }
   
   /// Method with a return value, two arguments
   template<typename C, typename A, typename B>
   C tfunc2(A a, B b)
   {
      std::cout << "TemplateClass<T>::tfunc2<A,B,C>(" << a << "," << b << ") called" << std::endl;
      numFunctionsCalled++;
      // C had better be a number type, or else this will fail to compile, but
      // this is just a demonstration and isn't meant to be robust
      return static_cast<C>(6.67e-11f);
   }
   
};

////////////////////////////////////////////////////////////////////////////////
// Test routines
////////////////////////////////////////////////////////////////////////////////

// Demonstrate getting and using function pointers
void pointerToFunction(void)
{
   // Get the pointer
   void (*ptr0)(void) = &func0;

   // Call the function
   (*ptr0)();
   
   // Get a pointer to a function that takes two arguments, then
   // call the function
   void (*ptr1)(int, float) = &func1;
   (*ptr1)(10, 3.14f);
   
   // Test two arguments and a return val
   float (*ptr2)(int, float) = &func2;
   float retval2 = (*ptr2)(50,11.0f);
   std::cout << "return from func2: " << retval2 << std::endl;
   
   void (*ptr3)(const int) = &func3;
   (*ptr3)(10);
}

// Demonstrate getting and using pointers to class methods
void pointerToMethod(void)
{
   // Create an instance of the class
   TestClass instance;
   
   // Get the pointer
   void (TestClass::*ptr0)(void) = &TestClass::func0;
   
   // Call the method
   (instance.*ptr0)();
   
   // Get a pointer to a function that takes two arguments, then
   // call the function
   void (TestClass::*ptr1)(int, float) = &TestClass::func1;
   (instance.*ptr1)(10, 3.14f);
   
   // Test two arguments and a return val
   float (TestClass::*ptr2)(int, float) = &TestClass::func2;
   float retval2 = (instance.*ptr2)(50,11.0f);
   std::cout << "return from TestClass::func2: " << retval2 << std::endl;
}

// Demonstrate getting and using function pointers to template functions
void pointerToTemplateFunction(void)
{
   // Get the pointer
   void (*ptr0)(void) = &tfunc0<char>;
   
   // Call the function
   (*ptr0)();
   
   // Get a pointer to a function that takes two arguments, then
   // call the function
   void (*ptr1)(int, float) = &tfunc1<int,float>;
   (*ptr1)(10, 3.14f);
   
   // Test two arguments and a return val
   float (*ptr2)(int, float) = &tfunc2<float, int, float>;
   float retval2 = (*ptr2)(50,11.0f);
   std::cout << "return from tfunc2: " << retval2 << std::endl;
}

// Demonstrate getting and using method pointers that are inside
// a templated class
void pointerToMethodInTemplateClass(void)
{
   // Create an instance of the class
   TemplateClass<long> instance;
   
   // Get the pointer
   void (TemplateClass<long>::*ptr0)(void) = &TemplateClass<long>::func0;
   
   // Call the method
   (instance.*ptr0)();
   
   // Get a pointer to a function that takes two arguments, then
   // call the function
   void (TemplateClass<long>::*ptr1)(int, float) = &TemplateClass<long>::func1;
   (instance.*ptr1)(10, 3.14f);
   
   // Test two arguments and a return val
   float (TemplateClass<long>::*ptr2)(int, float) = &TemplateClass<long>::func2;
   float retval2 = (instance.*ptr2)(50,11.0f);
   std::cout << "return from TemplateClass<long>::func2: " << retval2 << std::endl;
}

// Demonstrate getting and using templatized method pointers that are inside
// a templated class
void pointerToTemplateMethodInTemplateClass(void)
{
   // Create an instance of the class
   TemplateClass<long>* instance = new TemplateClass<long>;
   
   // Get the pointer
   void (TemplateClass<long>::*ptr0)(void) = &TemplateClass<long>::tfunc0<int>;
   
   // Call the method
   (instance->*ptr0)();
   
   // Get a pointer to a function that takes two arguments, then
   // call the function
   void (TemplateClass<long>::*ptr1)(int, float) = &TemplateClass<long>::tfunc1<int, float>;
   (instance->*ptr1)(10, 3.14f);
   
   // Test two arguments and a return val
   float (TemplateClass<long>::*ptr2)(int, float) = &TemplateClass<long>::tfunc2<float, int, float>;
   float retval2 = (instance->*ptr2)(50,11.0f);
   std::cout << "return from TemplateClass<long>::tfunc2: " << retval2 << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
// Entry point to program
/////////////////////////////////////////////////////////////////////////////////
int main(int, char**)
{ 
   // Run tests
   pointerToFunction();
   pointerToMethod();
   pointerToTemplateFunction();
   pointerToMethodInTemplateClass();
   pointerToTemplateMethodInTemplateClass();
   
   // Make sure all functions were called
   assert(numFunctionsCalled == 16);

   std::cout << "Tests passed" << std::endl;
   return 0;
}
