#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>

int main(int argc, char** argv){

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
        ("size,s", value<int>()->default_value(10), " size (if > 0, overrides m, n, k).")
        ("debug", value<std::string>()->default_value("no"), "(debug) => print matrices");

    variables_map vm;

    store(parse_command_line(argc, argv, desc_cmdline),vm);

    int s = vm["size"].as<int>();
    std::string out = vm["debug"].as<std::string>();

    bool debug = false;

    if(out == "yes"){
        debug = true;
    }

    double *A = new double[s];

    
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < s; i++){
//	A[i] = i * i + i;
	A[i] = rand() % 10 + 1;
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    std::cout << "For Loop C++ ]> " << ", size = " << s <<"; elapsed time: " << elapsed.count() << std::endl;


    return 0;
}
