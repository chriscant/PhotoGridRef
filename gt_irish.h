/*****************************************************************************
*
* GT_Irish holds Irish grid coordinates
* GT_WGS84 holds WGS84 latitude and longitude
* GT_Math is a collection of static methods doing all the nasty sums
*
* Converted to C++ from the code here: http://www.nearby.org.uk/tests/GeoTools2.html
*
*****************************************************************************/

#pragma once

#ifndef __GT_IRISH_H
#define __GT_IRISH_H

#include <string>

namespace PhotoGridRef {

  class GT_Irish {
  public:
    double northings, eastings, height;
    std::wstring status;

    GT_Irish();
    void setGridCoordinates(double _eastings, double _northings, double _height);
    os_grid_ref_t getGridRef();
    void setError(std::wstring msg);

  private:
    char prefixes[5][5] =
    { {'V', 'Q', 'L', 'F', 'A'},
      {'W', 'R', 'M', 'G', 'B'},
      {'X', 'S', 'N', 'H', 'C'},
      {'Y', 'T', 'O', 'J', 'D'},
      {'Z', 'U', 'P', 'K', 'E'} };
  };

  class GT_WGS84 {
  public:
    double latitude, longitude, height;
    GT_WGS84(double _latitude, double _longitude, double _height);
    bool isIreland();
    GT_Irish getIrish(bool uselevel2=FALSE);  /* FALSE seems to be consistent with irish.gridreferencefinder.com */
  };

  class GT_Math {
  public:
    static double Lat_Long_to_East(double PHI, double LAM, double a, double b, double e0, double f0, double PHI0, double LAM0);
    static double Lat_Long_to_North(double PHI, double LAM, double a, double b, double e0, double n0, double f0, double PHI0, double LAM0);
    static double Lat_Long_H_to_X(double PHI, double LAM, double H, double a, double b);
    static double Lat_Long_H_to_Y(double PHI, double LAM, double H, double a, double b);
    static double Lat_H_to_Z(double PHI, double H, double a, double b);
    static double Helmert_X(double X, double Y, double Z, double DX, double Y_Rot, double Z_Rot, double s);
    static double Helmert_Y(double X, double Y, double Z, double DY, double X_Rot, double Z_Rot, double s);
    static double Helmert_Z(double X, double Y, double Z, double DZ, double X_Rot, double Y_Rot, double s);
    static double XYZ_to_Lat(double X, double Y, double Z, double a, double b);
    static double Iterate_XYZ_to_Lat(double a, double e2, double PHI1, double Z, double RootXYSqr);
    static double XYZ_to_Long(double X, double Y);
    static double Marc(double bf0, double n, double PHI0, double PHI);
  };
}

#endif
