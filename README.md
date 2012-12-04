*Not ready for public use, but if you are interested and you will ask 'wise' questions you may contact me.*

About
-----

The thing started when I took over the coding of regulation of heating at my home. This includes wood boiler, solar collectors, two hot water tanks, radiators and includes HTML web control interface.
It is based on AVR uC. There are 10 1-Wire temperature sensors, 4 valves, and 4 pumps. uC is then connected to PC, where CherryPy server is running and directly communicating with uC. The best part about web interface is that it is automatically generated based on C code. Info about variables and functions is extracted from object file produced by GCC and you can automatically control and call them via web interface. Use you imagination to see how this can ease debugging and building of the final interface.

It was tested on custom made system which I use for regulation and Arduino for one other task. I don't see a reason why at least parts of the code could be used for other uCs.

Getting Started
---------------

You are on your own, but this will change if project get mature enough.

Contributing
------------

I am on my own, but this will change if project get mature enough.

License
-------

GPL, and this will NOT change even if project get mature enough.
