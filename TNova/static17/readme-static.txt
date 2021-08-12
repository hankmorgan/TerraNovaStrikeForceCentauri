Static 1.7
----------

General
-------
Static is a program for exporting and importing heightmaps to and from Terra Nova. You can export any TN level to a heightmap and open it any terrain generator. Add a few mountains and islands, then import the heightmap back into another TN level.

Features:
* Accurate heightmaps thanks to 16-bit colour depth
* Recreates the high-resolution map from the low-resolution map without loss of detail
* Gives a summary of map properties

The high-resolution heightmap shapes the terrain in the level you are allowed to walk on. The low-resolution map makes up the area between the high-resolution map and the horizon. While the low-res map is less detailed (25%), it spans a larger (400%) surface area than the high-res map does.


Usage
-----
First, make sure that you have copied all maps from the Terra Nova CD to the TNOVA\MAPS\ directory on your hard drive. Also remove the write protection from said files. Static is a command-line program. When you start the program from the command prompt, you will be asked for some parameters:

-Mapfile to open
	Type either an absolute (d:\tnova\maps\arena.res) or relative filename (maps\arena.res).
-Action to take
	Alternatives are [E]xport and [I]mport heightmap.

Exporting:	
	The heightmap tn.raw will be created in the same directory as static.exe is in. It is saved as a 16-bit 257*257 pixel RAW file, Intel byte order.

Static will show a summary of heightmap properties during export:
Height          (lowest to highest points on map)
Peak-to-peak    (difference between highest and lowest points)
Height scale	(how many of the maximum 4096 levels are used in map)
Tiles           (distinct points in the terrain. Also pixels in heightmap)
Area            (the surface area that the heightmap spans. As indicated by Terra Nova HUD)

Importing:
	The heightmap tn.raw must be inside the same directory as static.exe is in when importing. Static will ask for a scaling factor between 1 and 100% to be used for the heightmap. This factor varies between all maps, so experimentation is needed for best results.

Make a backup of your existing mapfile before importing in case something goes wrong!


Changes since 1.5 (unreleased)
------------------------------
-Geometric averaging replaced by bilinear interpolation
-Only exported map is the low-resolution map

Changes since 1.0
-----------------
-Command line parameters changed to queries
-Importing heightmaps into Terra Nova levels is now possible
-Output format changed to RAW due to limitation in Terragen
-Added "combined" heightmap support (low-resolution + high-resolution map)
-Considerably cleaner code


The fine print
--------------
Static is free to use. The source code in included, use it for your own project as long as you also release the code for it. Copyright 2008 Gigaquad.