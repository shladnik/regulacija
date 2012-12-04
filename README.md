About
-----

The thing started when I took over the coding of regulation of heating at my home. This includes wood boiler, solar collectors, two hot water tanks, radiators and includes HTML web control interface.
It is based on AVR uC. There are 10 1-Wire temperature sensors, 4 valves, and 4 pumps. uC is then connected to PC, where CherryPy server is running and directly communicating with uC. The best part about web interface is that it is automatically generated based on C code. Info about variables and functions is extracted from object file produced by GCC and you can automatically control and call them via web interface. Use you imagination to see how this can ease debugging and building of the final interface.

It was tested on custom made system which I use for regulation and Arduino for one other task. I don't see a reason why at least parts of the code could be used for other uCs.

Getting Started
---------------

*Beta - contact when you stuck*

You need:
- Arduino board
- avr-gcc toolchain with avr-libc (I use Gentoo with crossdev)
- avrdude
- pyserial
- cherrypy

Compile the code:

  ./make.py

Flash the flash (ideally you would do that only once, cause there is a web based bootloader available):

  avrdude -c stk500v2 -P /dev/ttyUSB0 -p m328p  -U flash:w:fw.bin

Set the fuses:

  ./py/fuse.py restore

Start the server:

  ./py/server.py

Connect to localhost:8000. I suggest you visit the flash page first and apply update.tar.bz2 created by make, cause bootloader was not updated by avrdude.


Contributing
------------

Is very welcome and that's why this project is here. If I would have all the time on the world I would code everything myself and I would be rich.

License
-------

GPL
