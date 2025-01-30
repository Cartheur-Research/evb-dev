#include "f64sim.hpp"

#include "math.h"
#include "float.h"


struct SimulationContext {
	bool verbose;

  SimulationContext()
  : verbose(false)
  { }

};




int main(int argc, char**argv)
{


	if (argc > 1) {
    SimulationContext cntxt;
		int i = 1;
		while (i<argc) {
			std::string opt(argv[i++]);
			if (cntxt.verbose) {std::cout << "arg[" << i-1 << "]:" << opt << std::endl;}
			if (!opt.empty()) {
				if (opt[0] == '-') {
					if (opt == "--verbose") {
						cntxt.verbose = true;
					}
					else if (opt == "-v") {
						cntxt.verbose = true;
					}
					else {
						std::cerr << "invalid option: " << opt << std::endl;
						//linker.error("invalid option "+opt);
					}
				}
			}
		}
    return 0;
  }
	std::cout << "usage:" << std::endl;
	std::cout << "\tf64sim [--verbose|-v] <executable>" << std::endl;
	return -1;
}
