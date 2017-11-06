#include <fstream>
#include <iostream>
#include <string>

#include "logging.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "AstPrinter.hpp"
#include "Interpreter.hpp"


namespace loxx
{
  void run(const std::string& src)
  {
    Scanner scanner(src);
    auto tokens = scanner.scan_tokens();

    for (const auto& token : tokens) {
      std::cout << token << '\n';
    }

    Parser parser(std::move(tokens));
    const auto expr = parser.parse();

    if (had_error) {
      return;
    }

    AstPrinter printer;

    std::cout << printer.print(*expr) << std::endl;

    static Interpreter interpreter;

    interpreter.interpret(*expr);
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
      std::exit(65);
    }
    else if (had_runtime_error) {
      std::exit(70);
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