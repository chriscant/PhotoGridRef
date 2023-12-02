PhotoGridRef
============

This tool displays a [GB](https://en.wikipedia.org/wiki/Ordnance_Survey_National_Grid) 
or [Irish](https://en.wikipedia.org/wiki/Irish_grid_reference_system)
Grid Reference for a JPG file in Windows, if the photo has suitable GPS information.
The Grid Reference is also copied to the clipboard, ready for pasting into a recording spreadsheet or website.

* Example GB grid reference: `NY 27332 33320`, copied to the clipboard as `NY2733233320`.
* Example Irish grid reference: `J 34386 74590`, copied to the clipboard as `J3438674590`.
* Latitude, Longitude and other details such as [tetrad](https://www.bto.org/volunteer-surveys/birdatlas/methods/correct-grid-references) are also listed.

The accuracy of the grid reference is primarily determined by the accuracy of the GPS reading in your phone or camera.
In many cases it may be best to express as a 6 figure grid reference eg `NY273333` ie by removing the last two digits of each group.

![PhotoGridRef screenshot](https://raw.githubusercontent.com/chriscant/PhotoGridRef/master/docs/PhotoGridRef-Screenshot1.png)

## Overview

* Once the tool is installed, right-click on a photo in Windows Explorer and select *Grid Ref*.
* If the Shift key is held down, then Eastings and Northings are found instead, eg `327332,533320`
* If the Control key is held down, the location is shown in the default browser at streetmap.co.uk instead (only works in GB)

## Instructions

* To use the tool [download and save PhotoGridRef.exe](https://github.com/chriscant/PhotoGridRef/raw/master/bin/PhotoGridRef.exe) - or [from here](http://phdcc.com/download/PhotoGridRef.exe)
* Save `PhotoGridRef.exe` on your computer.
* Double-click to install the context menu handler.
* After that, right-clicking on an image file will include *Grid Ref* on the context menu;
click on that; if the image contains GPS info, the grid reference is displayed and copied to the clipboard.

## Uninstallation

* To uninstall, hold down Shift when right-clicking on an image and select *Uninstall Grid Ref*;
  or double-click again on `PhotoGridRef.exe` in Windows Explorer.
* You can then delete the file `PhotoGridRef.exe`
* In Windows 7+ the Grid Ref context menu item may only disappear after a reboot

## Compatibility

* Can be installed by non-admin users
* Should work in Windows 7+, both 32 bit and 64 systems
* Tested in Windows 7 and Windows 10 64 bit
* This repository is the source code of a Microsoft Visual Studio Community 2022 project

## Used with thanks

* Mayank Lahiri's [easyexif](https://github.com/mayanklahiri/easyexif): A tiny ISO-compliant C++ EXIF parsing library. Copyright (c) 2010-2015 Mayank Lahiri
* Jonathan Heathcote's [OS Coord](https://github.com/mossblaser/os_coord): A Simple OS Coordinate Transformation Library for C - with designated initializers removed
* Paul Dixon's [GeoTools javascript coordinate transformations](http://www.nearby.org.uk/tests/GeoTools2.html)
* More info available on Roger Muggleton's page at http://www.carabus.co.uk/ll_ngr.html
* netalloy's [map pin icon](https://openclipart.org/detail/169839/map-pin) and [ICO Convert](http://icoconvert.com/)

## Ideas

* Support worldwide [Maidenhead Locator System](https://en.wikipedia.org/wiki/Maidenhead_Locator_System) eg using this perl code [Ham::Locator](https://metacpan.org/pod/release/MEH/Ham-Locator-0.1000/lib/Ham/Locator.pm)
* Display found data in selectable form
* Add icon and about box to displayed window

## License

* [License (BSD-3-Clause)](LICENSE) BSD. Free for personal and commercial use.
