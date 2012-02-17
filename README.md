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

Attribution-NonCommercial-ShareAlike 3.0  (CC BY-NC-SA 3.0)

*	**You are free:**


	* to Share - to copy, distribute and transmit the work
	* to Remix - to adapt the work

*	**Under the following conditions:**

	* Attribution - You must attribute the work in the manner specified by the author or licensor
	(but not in any way that suggests that they endorse you or your use of the work).
	* Noncommercial - You may not use this work for commercial purposes.
	* Share Alike - If you alter, transform, or build upon this work, you may distribute the resulting
	work only under the same or similar license to this one.

*	**With the understanding that:**

	* Waiver - Any of the above conditions can be waived if you get permission from the copyright holder.
	* Public Domain - Where the work or any of its elements is in the public domain under applicable law,
	that status is in no way affected by the license.
	* Other Rights - In no way are any of the following rights affected by the license:
		- Your fair dealing or fair use rights, or other applicable copyright exceptions and limitations;
		- The author's moral rights;
		- Rights other persons may have either in the work itself or in how the work is used, such as publicity or privacy rights.

For more details about the license visit [http://creativecommons.org/licenses/by-nc-sa/3.0/](http://creativecommons.org/licenses/by-nc-sa/3.0/)

