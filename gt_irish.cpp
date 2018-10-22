/*****************************************************************************
*
* GT_Irish holds Irish grid coordinates
* GT_WGS84 holds WGS84 latitude and longitude
* GT_Math is a collection of static methods doing all the nasty sums
*
* Converted to C++ from the code here: http://www.nearby.org.uk/tests/GeoTools2.html
*
*****************************************************************************/

#include "stdafx.h"
#include "os_coord.h"
#include "gt_irish.h"

namespace PhotoGridRef {

  /*****************************************************************************
  *
  * GT_OSGB holds OSGB grid coordinates
  *
  *****************************************************************************/

  GT_Irish::GT_Irish() {
    northings = 0;
    eastings = 0;
    status = L"Undefined";
  }

  void GT_Irish::setGridCoordinates(double _eastings, double _northings, double _height)
  {
    northings = _northings;
    eastings = _eastings;
    height = _height;
    status = L"OK";
  }

  os_grid_ref_t GT_Irish::getGridRef()
  {
    // An invalid grid reference (to start with)
    os_grid_ref_t grid_ref;

    int y = (int)(floor(northings / 100000));
    int x = (int)(floor(eastings / 100000));

    grid_ref.code[0] = prefixes[x][y];
    grid_ref.code[1] = L'\0';
    grid_ref.e = fmod(eastings, 100000.0);
    grid_ref.n = fmod(northings, 100000.0);
    grid_ref.h = height;

    return grid_ref;
  }

  void GT_Irish::setError(std::wstring msg) {
    status = msg;
  }

  /*****************************************************************************
  *
  * GT_WGS84 holds WGS84 latitude and longitude
  *
  *****************************************************************************/

  GT_WGS84::GT_WGS84(double _latitude, double _longitude, double _height) {
    latitude = _latitude;
    longitude = _longitude;
    height = _height;
  }

  bool GT_WGS84::isIreland()
  {
    return latitude > 51.2 &&
      latitude < 55.73 &&
      longitude > -12.2 &&
      longitude < -4.8;
  }

  GT_Irish GT_WGS84::getIrish(bool uselevel2) /* FALSE seems to be consistent with irish.gridreferencefinder.com */
  {
    GT_Irish irish;
    if (isIreland())
    {
      double height = 0;

      double latitude2, longitude2;
      if (uselevel2) {
        double x1 = GT_Math::Lat_Long_H_to_X(latitude, longitude, height, 6378137.00, 6356752.313);
        double y1 = GT_Math::Lat_Long_H_to_Y(latitude, longitude, height, 6378137.00, 6356752.313);
        double z1 = GT_Math::Lat_H_to_Z(latitude, height, 6378137.00, 6356752.313);

        double x2 = GT_Math::Helmert_X(x1, y1, z1, -482.53, -0.214, -0.631, -8.15);
        double y2 = GT_Math::Helmert_Y(x1, y1, z1, 130.596, -1.042, -0.631, -8.15);
        double z2 = GT_Math::Helmert_Z(x1, y1, z1, -564.557, -1.042, -0.214, -8.15);

        latitude2 = GT_Math::XYZ_to_Lat(x2, y2, z2, 6377340.189, 6356034.447);
        longitude2 = GT_Math::XYZ_to_Long(x2, y2);
      }
      else {
        latitude2 = latitude;
        longitude2 = longitude;
      }

      double e = GT_Math::Lat_Long_to_East(latitude2, longitude2, 6377340.189, 6356034.447, 200000, 1.000035, 53.50000, -8.00000);
      double n = GT_Math::Lat_Long_to_North(latitude2, longitude2, 6377340.189, 6356034.447, 200000, 250000, 1.000035, 53.50000, -8.00000);

      if (!uselevel2) {
        //Level 1 Transformation - 95% of points within 2 metres
        //fixed datum shift correction (instead of fancy hermert translation above!)
        //source http://www.osni.gov.uk/downloads/Making%20maps%20GPS%20compatible.pdf
        e = e + 49;
        n = n - 23.4;
      }

      irish.setGridCoordinates(round(e), round(n), height);
    }
    else
    {
      irish.setError(L"Coordinate not within Ireland");
    }

    return irish;
  }

  /*****************************************************************************
  *
  * GT_Math is a collection of static methods doing all the nasty sums
  *
  *****************************************************************************/

  double GT_Math::Lat_Long_to_East(double PHI, double LAM, double a, double b, double e0, double f0, double PHI0, double LAM0)
  {
    //Project Latitude and longitude to Transverse Mercator eastings.
    //Input: - _
    //    Latitude (PHI) and Longitude (LAM) in decimal degrees; _
    //    ellipsoid axis dimensions (a & b) in meters; _
    //    eastings of false origin (e0) in meters; _
    //    central meridian scale factor (f0); _
    // latitude (PHI0) and longitude (LAM0) of false origin in decimal degrees.

    // Convert angle measures to radians
    double Pi = 3.14159265358979;
    double RadPHI = PHI * (Pi / 180);
    double RadLAM = LAM * (Pi / 180);
    double RadPHI0 = PHI0 * (Pi / 180);
    double RadLAM0 = LAM0 * (Pi / 180);

    double af0 = a * f0;
    double bf0 = b * f0;
    double e2 = (pow(af0, 2) - pow(bf0, 2)) / pow(af0, 2);
    double n = (af0 - bf0) / (af0 + bf0);
    double nu = af0 / (sqrt(1 - (e2 * pow(sin(RadPHI), 2))));
    double rho = (nu * (1 - e2)) / (1 - (e2 * pow(sin(RadPHI), 2)));
    double eta2 = (nu / rho) - 1;
    double p = RadLAM - RadLAM0;

    double IV = nu * (cos(RadPHI));
    double V = (nu / 6) * (pow(cos(RadPHI), 3)) * ((nu / rho) - (pow(tan(RadPHI), 2)));
    double VI = (nu / 120) * (pow(cos(RadPHI), 5)) * (5 - (18 * (pow(tan(RadPHI), 2))) + (pow(tan(RadPHI), 4)) + (14 * eta2) - (58 * (pow(tan(RadPHI), 2)) * eta2));

    return e0 + (p * IV) + (pow(p, 3) * V) + (pow(p, 5) * VI);
  }
  
  double GT_Math::Lat_Long_to_North(double PHI, double LAM, double a, double b, double e0, double n0, double f0, double PHI0, double LAM0) {
    // Project Latitude and longitude to Transverse Mercator northings
    // Input: - _
    // Latitude (PHI) and Longitude (LAM) in decimal degrees; _
    // ellipsoid axis dimensions (a & b) in meters; _
    // eastings (e0) and northings (n0) of false origin in meters; _
    // central meridian scale factor (f0); _
    // latitude (PHI0) and longitude (LAM0) of false origin in decimal degrees.

    // REQUIRES THE "Marc" FUNCTION

    // Convert angle measures to radians
    double Pi = 3.14159265358979;
    double RadPHI = PHI * (Pi / 180);
    double RadLAM = LAM * (Pi / 180);
    double RadPHI0 = PHI0 * (Pi / 180);
    double RadLAM0 = LAM0 * (Pi / 180);

    double af0 = a * f0;
    double bf0 = b * f0;
    double e2 = (pow(af0, 2) - pow(bf0, 2)) / pow(af0, 2);
    double n = (af0 - bf0) / (af0 + bf0);
    double nu = af0 / (sqrt(1 - (e2 * pow(sin(RadPHI), 2))));
    double rho = (nu * (1 - e2)) / (1 - (e2 * pow(sin(RadPHI), 2)));
    double eta2 = (nu / rho) - 1;
    double p = RadLAM - RadLAM0;
    double M = GT_Math::Marc(bf0, n, RadPHI0, RadPHI);

    double I = M + n0;
    double II = (nu / 2) * (sin(RadPHI)) * (cos(RadPHI));
    double III = ((nu / 24) * (sin(RadPHI)) * (pow(cos(RadPHI), 3))) * (5 - (pow(tan(RadPHI), 2)) + (9 * eta2));
    double IIIA = ((nu / 720) * (sin(RadPHI)) * (pow(cos(RadPHI), 5))) * (61 - (58 * (pow(tan(RadPHI), 2))) + (pow(tan(RadPHI), 4)));

    return I + (pow(p, 2) * II) + (pow(p, 4) * III) + (pow(p, 6) * IIIA);
  }

  double GT_Math::Lat_Long_H_to_X(double PHI, double LAM, double H, double a, double b) {
    // Convert geodetic coords lat (PHI), long (LAM) and height (H) to cartesian X coordinate.
    // Input: - _
    //    Latitude (PHI)& Longitude (LAM) both in decimal degrees; _
    //  Ellipsoidal height (H) and ellipsoid axis dimensions (a & b) all in meters.

    // Convert angle measures to radians
    double Pi = 3.14159265358979;
    double RadPHI = PHI * (Pi / 180);
    double RadLAM = LAM * (Pi / 180);

    // Compute eccentricity squared and nu
    double e2 = (pow(a, 2) - pow(b, 2)) / pow(a, 2);
    double V = a / (sqrt(1 - (e2 * (pow(sin(RadPHI), 2)))));

    // Compute X
    return (V + H) * (cos(RadPHI)) * (cos(RadLAM));
  }

  double GT_Math::Lat_Long_H_to_Y(double PHI, double LAM, double H, double a, double b) {
    // Convert geodetic coords lat (PHI), long (LAM) and height (H) to cartesian Y coordinate.
    // Input: - _
    // Latitude (PHI)& Longitude (LAM) both in decimal degrees; _
    // Ellipsoidal height (H) and ellipsoid axis dimensions (a & b) all in meters.

    // Convert angle measures to radians
    double Pi = 3.14159265358979;
    double RadPHI = PHI * (Pi / 180);
    double RadLAM = LAM * (Pi / 180);

    // Compute eccentricity squared and nu
    double e2 = (pow(a, 2) - pow(b, 2)) / pow(a, 2);
    double V = a / (sqrt(1 - (e2 * (pow(sin(RadPHI), 2)))));

    // Compute Y
    return (V + H) * (cos(RadPHI)) * (sin(RadLAM));

  }

  double GT_Math::Lat_H_to_Z(double PHI, double H, double a, double b) {
    // Convert geodetic coord components latitude (PHI) and height (H) to cartesian Z coordinate.
    // Input: - _
    //    Latitude (PHI) decimal degrees; _
    // Ellipsoidal height (H) and ellipsoid axis dimensions (a & b) all in meters.

    // Convert angle measures to radians
    double Pi = 3.14159265358979;
    double RadPHI = PHI * (Pi / 180);

    // Compute eccentricity squared and nu
    double e2 = (pow(a, 2) - pow(b, 2)) / pow(a, 2);
    double V = a / (sqrt(1 - (e2 * (pow(sin(RadPHI), 2)))));

    // Compute X
    return ((V * (1 - e2)) + H) * (sin(RadPHI));
  }

  double GT_Math::Helmert_X(double X, double Y, double Z, double DX, double Y_Rot, double Z_Rot, double s) {
    // (X, Y, Z, DX, Y_Rot, Z_Rot, s)
    // Computed Helmert transformed X coordinate.
    // Input: - _
    //    cartesian XYZ coords (X,Y,Z), X translation (DX) all in meters ; _
    // Y and Z rotations in seconds of arc (Y_Rot, Z_Rot) and scale in ppm (s).

    // Convert rotations to radians and ppm scale to a factor
    double Pi = 3.14159265358979;
    double sfactor = s * 0.000001;

    double RadY_Rot = (Y_Rot / 3600) * (Pi / 180);

    double RadZ_Rot = (Z_Rot / 3600) * (Pi / 180);

    //Compute transformed X coord
    return  (X + (X * sfactor) - (Y * RadZ_Rot) + (Z * RadY_Rot) + DX);
  }

  double GT_Math::Helmert_Y(double X, double Y, double Z, double DY, double X_Rot, double Z_Rot, double s) {
    // (X, Y, Z, DY, X_Rot, Z_Rot, s)
    // Computed Helmert transformed Y coordinate.
    // Input: - _
    //    cartesian XYZ coords (X,Y,Z), Y translation (DY) all in meters ; _
    //  X and Z rotations in seconds of arc (X_Rot, Z_Rot) and scale in ppm (s).

    // Convert rotations to radians and ppm scale to a factor
    double Pi = 3.14159265358979;
    double sfactor = s * 0.000001;
    double RadX_Rot = (X_Rot / 3600) * (Pi / 180);
    double RadZ_Rot = (Z_Rot / 3600) * (Pi / 180);

    // Compute transformed Y coord
    return (X * RadZ_Rot) + Y + (Y * sfactor) - (Z * RadX_Rot) + DY;
  }

  double GT_Math::Helmert_Z(double X, double Y, double Z, double DZ, double X_Rot, double Y_Rot, double s) {
    // (X, Y, Z, DZ, X_Rot, Y_Rot, s)
    // Computed Helmert transformed Z coordinate.
    // Input: - _
    //    cartesian XYZ coords (X,Y,Z), Z translation (DZ) all in meters ; _
    // X and Y rotations in seconds of arc (X_Rot, Y_Rot) and scale in ppm (s).
    // 
    // Convert rotations to radians and ppm scale to a factor
    double Pi = 3.14159265358979;
    double sfactor = s * 0.000001;
    double RadX_Rot = (X_Rot / 3600) * (Pi / 180);
    double RadY_Rot = (Y_Rot / 3600) * (Pi / 180);

    // Compute transformed Z coord
    return (-1 * X * RadY_Rot) + (Y * RadX_Rot) + Z + (Z * sfactor) + DZ;
  }

  double GT_Math::XYZ_to_Lat(double X, double Y, double Z, double a, double b) {
    // Convert XYZ to Latitude (PHI) in Dec Degrees.
    // Input: - _
    // XYZ cartesian coords (X,Y,Z) and ellipsoid axis dimensions (a & b), all in meters.

    // THIS FUNCTION REQUIRES THE "Iterate_XYZ_to_Lat" FUNCTION
    // THIS FUNCTION IS CALLED BY THE "XYZ_to_H" FUNCTION

    double RootXYSqr = sqrt(pow(X, 2) + pow(Y, 2));
    double e2 = (pow(a, 2) - pow(b, 2)) / pow(a, 2);
    double PHI1 = atan2(Z, (RootXYSqr * (1 - e2)));

    double PHI = GT_Math::Iterate_XYZ_to_Lat(a, e2, PHI1, Z, RootXYSqr);

    double Pi = 3.14159265358979;

    return PHI * (180 / Pi);
  }

  double GT_Math::Iterate_XYZ_to_Lat(double a, double e2, double PHI1, double Z, double RootXYSqr) {
    // Iteratively computes Latitude (PHI).
    // Input: - _
    //    ellipsoid semi major axis (a) in meters; _
    //    eta squared (e2); _
    //    estimated value for latitude (PHI1) in radians; _
    //    cartesian Z coordinate (Z) in meters; _
    // RootXYSqr computed from X & Y in meters.

    // THIS FUNCTION IS CALLED BY THE "XYZ_to_PHI" FUNCTION
    // THIS FUNCTION IS ALSO USED ON IT'S OWN IN THE _
    // "Projection and Transformation Calculations.xls" SPREADSHEET


    double V = a / (sqrt(1 - (e2 * pow(sin(PHI1), 2))));
    double PHI2 = atan2((Z + (e2 * V * (sin(PHI1)))), RootXYSqr);

    while (abs(PHI1 - PHI2) > 0.000000001) {
      PHI1 = PHI2;
      V = a / (sqrt(1 - (e2 * pow(sin(PHI1), 2))));
      PHI2 = atan2((Z + (e2 * V * (sin(PHI1)))), RootXYSqr);
    }

    return PHI2;
  }

  double GT_Math::XYZ_to_Long(double X, double Y) {
    // Convert XYZ to Longitude (LAM) in Dec Degrees.
    // Input: - _
    // X and Y cartesian coords in meters.

    double Pi = 3.14159265358979;
    return atan2(Y, X) * (180 / Pi);
  }

  double GT_Math::Marc(double bf0, double n, double PHI0, double PHI) {
    //Compute meridional arc.
    //Input: - _
    // ellipsoid semi major axis multiplied by central meridian scale factor (bf0) in meters; _
    // n (computed from a, b and f0); _
    // lat of false origin (PHI0) and initial or final latitude of point (PHI) IN RADIANS.

    //THIS FUNCTION IS CALLED BY THE - _
    // "Lat_Long_to_North" and "InitialLat" FUNCTIONS
    // THIS FUNCTION IS ALSO USED ON IT'S OWN IN THE "Projection and Transformation Calculations.xls" SPREADSHEET

    return bf0 * (((1 + n + ((5 / 4) * pow(n, 2)) + ((5 / 4) * pow(n, 3))) * (PHI - PHI0)) - (((3 * n) + (3 * pow(n, 2)) + ((21 / 8) * pow(n, 3))) * (sin(PHI - PHI0)) * (cos(PHI + PHI0))) + ((((15 / 8      ) * pow(n, 2)) + ((15 / 8) * pow(n, 3))) * (sin(2 * (PHI - PHI0))) * (cos(2 * (PHI + PHI0)))) - (((35 / 24) * pow(n, 3)) * (sin(3 * (PHI - PHI0))) * (cos(3 * (PHI + PHI0)))));
  }

};
