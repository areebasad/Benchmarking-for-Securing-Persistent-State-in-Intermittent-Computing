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


Below are links to the published paper, pitch video, and presentation video.

## Resources

- **Published Paper:** [On Securing Persistent State in Intermittent Computing](https://dl.acm.org/doi/abs/10.1145/3417308.3430267)  
  Published in the ACM Digital Library.

- **Pitch Video:** [On Securing Persistent State in Intermittent Computing - Pitch](https://youtu.be/jugHNBL4q34?si=t0h-f5gLCC8dkowj)  
  A brief pitch video.

- **Presentation Video:** [On Securing Persistent State in Intermittent Computing - Presentation](https://youtu.be/qVcJtpyNWyk?si=mUpy59x4oDjAeOuU)  
  A detailed presentation covering the motivation, threat models, results, and implications of the research.


## Citation

If you use this work, please cite it as:
```bibtex
@inproceedings{asad2020securing,
  title={On securing persistent state in intermittent computing},
  author={Asad, Hafiz Areeb and Wouters, Erik Henricus and Bhatti, Naveed Anwar and Mottola, Luca and Voigt, Thiemo},
  booktitle={Proceedings of the 8th International Workshop on Energy Harvesting and Energy-Neutral Sensing Systems},
  pages={8--14},
  year={2020}
}

