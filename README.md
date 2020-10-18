# Benchmarking-for-Securing-Persistent-State-in-Intermittent-Computing


This repository includes the code for benchmarking different security mechanisms to protect
persistent state in intermittent computing. The code for benchmarks run directly on bare-metal without operating
system support. They emulate the occurrence of checkpoint and
restore operations, to measure the overhead of different security
mechanisms.


## For getting started:

Install PyDGILib, 
PyDGILib provides python bindings for Atmel® Data Gateway Interface (DGI) devices and allows collecting and visualizing power usage data.
The live version of the documentation can be found at https://ewouters.github.io/pydgilib/.
The documentation is very extensive and while the getting started example
shows an easy way to get going, it should enable people to use the full extend of the library.
Download librray from https://pypi.org/project/pydgilib/#description

The repository includes benchmarking security code for SAML11 (Cortex-M23) and SAML21 (Cortex-M0+) 

Authors: 
Erik Henricus Wouters,
Hafiz Areeb Asad,
Luca Mottola,
Naveed Anwar Bhatti,
Thiemo Voigt
