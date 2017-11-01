#include <fstream>
#include <iostream>
#include <string>

#include "logging.hpp"
#include "Scanner.hpp"


namespace loxx
{
  void run(const std::string& src)
  {
    Scanner scanner(src);
    const auto& tokens = scanner.scan_tokens();

    for (const auto& token : tokens) {
      std::cout << token << '\n';
    }
  }


  void run_prompt()
  {
    while (true) {
      std::cout << "> ";
      std::string src;
      std::getline(std::cin, src);
      run(src);
      had_error = false;
    }
  }


  void run_file(const std::string& path)
  {
    std::ifstream file(path);
    std::streamsize size = file.tellg();

    file.seekg(0, std::ios::beg);

    std::string src(static_cast<unsigned long>(size), '\0');
    if (!file.read(&src[0], size)) {
      throw std::ios_base::failure("Unable to read source file!");
    }

    run(src);

    if (had_error) {
      throw std::runtime_error("Error encountered when executing file!");
    }
  }
}


int main(int argc, char* argv[])
{
  if (argc == 2) {
    const std::string file_path(argv[1]);
    loxx::run_file(file_path);
  }
  else {
    loxx::run_prompt();
  }
}