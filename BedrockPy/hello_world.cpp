#include<iostream>

#include <Python.h>
#include <pybind11/pybind11.h>

using namespace std;

int main(int argc, char** argv)
{
  cout << "hello, world" << endl;
#ifdef PRINTA
  cout << PRINTA << endl;
#endif

#ifdef PRINTB
  cout << PRINTB << endl;
#endif
  return 0;
}
