#include "studio.hpp"

#include "math.h"
#include "float.h"

int main(int argc, char**argv)
{
	if (argc > 1) {
		std::string config_file(argv[1]);

    Gtk::Main appl(argc, argv);
    studio::MainWindow window;
    window.setup(config_file);
    appl.run(window);
    return 0;
  }
	std::cout << "usage:" << std::endl;
	std::cout << "\tgaide <ide-file>" << std::endl;
	return -1;
}
