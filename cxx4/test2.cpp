// Purpose: Converts ida3 format xma data to netcdf4
// Usage:   xma2netcdf <shot number>


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstddef>

using namespace std;

int main()
{
struct st{
  double x;
  int y;};
 
 struct st a;
  cout <<a.x <<endl;
  cout <<a.y <<endl;
  cout << offsetof(st,y) <<endl;
  return 1;
}

