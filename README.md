Doser
=======

`Doser` is an Arduino (RBBB) based 5 Channel Doser and Auto Top-Off controller by Georgi Todorov.

Features
--------

*	**5 Channels**

	`Doser` has 5 channels for peristaltic pumps that can be individually 
        calibrated and set to precise schedule for dosing, automatic or continuous water changes.

*	**ATO**

	`Doser` can also handle auto top-off needs using simple float switches.
	The controller can be set to work with 1, 2 or 3 switch configuration.
	It is recommended that at least 2 float switches are used to ensure that
	your top off pump does not run out of water and burn out.

	The ATO system will have a safety timer to prevent accidental overdosing of fresh water.

*	**RTC**
	
	`Doser` has a real time clock chip with battery backup that keeps the time
	for accurate scheduling of top-off and dosing.

*	**LCD**

	`Doser` uses a 4x20 character LCD to display the current date and time as well as
	dosing information and error states from the top-off system.

*	**IR Remote Control**

	`Doser` also has an infrared sensor that is used for use input and settings. 
	The controller should be able to learn almost any remote control and assign
	the required commands to any button.

*	**Really Bare Bones Board (RBBB) Arduino**

	`Doser` is using a RBBB Arduino from [Modern Device](http://shop.moderndevice.com/products/rbbb-kit)
	for its brain. 

Files
-------

*	**Software**
	
	The firmware for `Doser` is being developed using the [Arduino IDE](http://arduino.cc/en/Main/Software) 
	The code is Arduino 1.0 compatible and should work with older versions as well.
	The only required additional library is the [DS1302](http://www.henningkarlsen.com/electronics/library.php?id=5).

	Please note that the firmware is still a work in progress.

*	**Hardware**

	Under `hardware` you will find files from the free version of [Eagle PCB](http://www.cadsoftusa.com).

Getting a `Doser`
-------

*	**PCB**

	There are plenty fab house in China that offer PCB services for very reasonable price.
	[Seeed Studio](http://www.seeedstudio.com/depot/fusion-pcb-service-p-835.html?cPath=185) 
	will make 10 boards for a little over $25.
	[iTead Studio](http://iteadstudio.com/store/) has very similar offer.

*	**Dosing Pumps**

	`Doser` will work with almost any 12V dosing head. The controller was designed and tested with
	the Marine Magic dosing heads from [eBay](http://ebay.com).

*	**Bill Of Materials**

	The BOM files are under `hardware` in the repository. They list the rest of the required hardware
	including the [RBBB from Modern Device](http://shop.moderndevice.com/products/rbbb-kit)
	and the [LCD from Modern Device](http://shop.moderndevice.com/products/20x4-blue-lcd).
	The latest parts list, excluding LCD and RBBB can also be found on
	[Mouser](http://www.mouser.com:80/ProjectManager/ProjectDetail.aspx?AccessID=1ade37f4b7).


License
-------

*	**Software**

	Copyright (C) 2011-2012 Georgi Todorov
	All rights reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


*	**Hardware**
	Licensed under the TAPR Open Hardware License (www.tapr.org/OHL)
