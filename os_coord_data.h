/**
 * OS Coord: A Simple OS Coordinate Transformation Library for C
 *
 * This is a port of a the Javascript library produced by Chris Veness available
 * from http://www.movable-type.co.uk/scripts/latlong-gridref.html.
 *
 * Useful data/constants for the UK.
 */
#ifndef OS_COORD_DATA_H
#define OS_COORD_DATA_H

  /******************************************************************************
   * Ellipsoid definitions. Values taken from "A guide to coordinate systems in
   * Great Britain"
   ******************************************************************************/

   // Airy 1830
  const os_ellipsoid_t OS_EL_AIRY_1830 = { 6377563.396, 6356256.910 };

  // Airy 1830 Modified
  const os_ellipsoid_t OS_EL_AIRY_1830_MODIFIED = { 6377340.189,6356034.447 };

  // Internaltional 1923 (aka Hayford 1909)
  const os_ellipsoid_t OS_EL_INTERNATIONAL_1924 = { 6378388.000, 6356911.946 };

  // GRS80 (aka WGS84 ellipsoid)
  const os_ellipsoid_t OS_EL_WGS84 = { 6378137.000, 6356752.3141 };


  /******************************************************************************
   * Helmert transformations
   ******************************************************************************/

   // WGS84 -> OSGB36. Produces heights "similar to" ODN heights. From from "A
   // guide to coordinate systems in Great Britain".
  const os_helmert_t OS_HE_WGS84_TO_OSGB36 = {
    -446.448,125.157,-542.060,
    -0.1502,-0.2470,-0.8421,
     20.4894
  };

  // WGS84 -> ED50. Values taken from
  // http://og.decc.gov.uk/en/olgs/cms/pons_and_cop/pons/pon4/pon4.aspx
  const os_helmert_t OS_HE_WGS84_TO_ED50 = {
    89.5,93.8,123.1,
    0.0,0.0,0.156,
     -1.2
  };

  // ETAF89 (similar to WGS84) -> IRL 1975. Values taken from
  // http://www.osi.ie/OSI/media/OSI/Content/Publications/transformations_booklet.pdf
  // and
  // http://www.ordnancesurvey.co.uk/oswebsite/gps/information/coordinatesystemsinfo/guidecontents/guide6.html#6.5
  const os_helmert_t OS_HE_ETRF89_TO_IRL1975 = {
    -482.530,130.596,-564.557,
    -1.042,-0.214,-0.631,
     -8.150
  };


  /******************************************************************************
   * Transverse Mercator Projections. Values taken from "A guide to coordinate
   * systems in Great Britain".
   ******************************************************************************/


   // Ordenance Survey National Grid
  const os_tm_projection_t OS_TM_NATIONAL_GRID = {
    400000.0,-100000.0,
    0.9996012717,
    49.0,-2.0,
    {6377563.396,6356256.910} // Airy 1830
  };

  // Ordenance Survey Irish National Grid
  const os_tm_projection_t OS_TM_IRISH_NATIONAL_GRID = {
    200000.0,250000.0,
    1.000035,
    53.5,-8.0,
    {6377340.189,6356034.447} // Airy 1830 Modified
  };

  // Universal Transverse Mercator (UTM) Zone 29
  const os_tm_projection_t OS_TM_UTM_ZOME_29 = {
    500000.0,0.0,
    0.9996,
    0,-9.0,
    {6378388.000,6356911.946} // International 1924
  };

  // Universal Transverse Mercator (UTM) Zone 30
  const os_tm_projection_t OS_TM_UTM_ZOME_30 = {
    500000.0,0.0,
    0.9996,
    0,-3.0,
    {6378388.000,6356911.946} // International 1924
  };

  // Universal Transverse Mercator (UTM) Zone 31
  const os_tm_projection_t OS_TM_UTM_ZOME_31 = {
    500000.0,0.0,
    0.9996,
    0,3.0,
    {6378388.000,6356911.946} // International 1924
  };


  /******************************************************************************
   * National grid system grids.
   ******************************************************************************/

   // Ordinance Survey National Grid over England, Scotland and Wales.
  const os_grid_t OS_GR_NATIONAL_GRID = {
    2,
    'S',
    7,
    13
  };

  // Ordinance Survey National Grid over Ireland and Northern Ireland
  const os_grid_t OS_GR_IRISH_NATIONAL_GRID = {
    1,
    'V',
    5,
    5
  };

#endif

