PhotoGridRef
============

This tool displays a ten figure UK Grid Reference for a JPG file in Windows, if the photo has suitable GPS information.
The Grid Reference is also copied to the clipboard, ready for pasting into a recording spreadsheet or website.
Example grid reference: `NY 27332 33320`

The accuracy of the grid reference is primarily determined by the accuracy of the GPS reading in your phone or camera.
In many cases it may be best to express as a 6 figure grid reference eg `NY273333` ie by removing the last two digits of each group.

Overview:
* Once the tool is installed, right-click on a photo in Windows Explorer and select *Grid Ref*.
* If the Shift key is held down, then Eastings and Northings are found instead, eg `327332,533320`
* If the Control key is held down, the location is shown in the default browser at streetmap.co.uk instead

Instructions:
* To use the tool [download and save PhotoGridRef.exe](bin/PhotoGridRef.exe)
* Save `PhotoGridRef.exe` on your computer.
* Double-click to install the context menu handler.
* After that, right-clicking on an image file will include *Grid Ref* on the context menu;
click on that; if the image contains GPS info, the grid reference is displayed and copied to the clipboard.

Uninstallation:
* To uninstall, hold down Shift when right-clicking on an image and select *Uninstall Grid Ref*;
  or double-click again on `PhotoGridRef.exe` in Windows Explorer.
* You can then delete the file `PhotoGridRef.exe`
* In Windows 7 the Grid Ref context menu item may only disappear after a reboot

Compatibility:
* Can be installed by non-admin users
* Should work in Windows 7+, both 32 bit and 64 systems
* Tested in Windows 7 and Windows 10 64 bit
* This repository is the source code of a Microsoft Visual Studio Community 2017 project

### Used with thanks:

  * Mayank Lahiri's [easyexif](https://github.com/mayanklahiri/easyexif): A tiny ISO-compliant C++ EXIF parsing library. Copyright (c) 2010-2015 Mayank Lahiri
  * Jonathan Heathcote's [OS Coord](https://github.com/mossblaser/os_coord): A Simple OS Coordinate Transformation Library for C - with designated initializers removed
  * netalloy's [map pin icon](https://openclipart.org/detail/169839/map-pin) and [ICO Convert](http://icoconvert.com/)

### License

* [License (BSD-3-Clause)](LICENSE) BSD. Free for personal and commercial use.

