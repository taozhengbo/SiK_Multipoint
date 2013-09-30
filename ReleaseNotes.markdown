#Release Notes:

##MP SiK 2.3:

###Bug Fixes
1. Watchdog turned off due to odd behaviour
2. RTI5 returns full list of commands (Watchdog problem)
3. RT&W should be stable now (Watchdog problem)
4. Tweeking Radio timing to improve preformace

###NEW FEATURES!!

1. ATI8 Returns if the node is sync'd with the base (Does not work on base node)

2. Users can now controll unused pins. This can be preformed by the following commands

Command       | Function | Description
------------- | ---------|-------------
ATPP          | Print    | Print All Pins Settings
ATPI=1        | Input    | Set Pin 1 to Input
ATPR=1        | Read     | Read Pin 1 value (When set to input)
ATPO=2        | Output   | Set Pin 2 to Output (Output's by Default can only be controlled by AT cmd)
ATPC=2,1      | Control  | Turn pin 2 on  - Output Mode / Set internal pull up resistor - Input Mode 
ATPC=2,0      | Control  | Turn pin 2 off - Output Mode / Set internal pull down resistor - Input Mode

The mapping between the pin numbers above and the port number are below

Pin  | Port
---- | ----
0    | 2.3
1    | 2.2
2    | 2.1
3    | 2.0
4    | 2.6
5    | 0.1

##MP SiK 2.2:
###Lookup Changes

##MP SiK 2.1:
###First Release SiK MultiPoint Firmware

###NEW FEATURES!!
1. Support for Multinode Networks (more than 2!)
2. Improved Radio stack to minimise packet delays