#include <fstream>
#include <iostream>
#include <string>

#include <ezOptionParser.hpp>

#include "logging.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "AstPrinter.hpp"
#include "Interpreter.hpp"


namespace loxx
{
  void run(const std::string& src, const bool debug)
  {
    Scanner scanner(src);
    auto tokens = scanner.scan_tokens();

    if (debug) {
      for (const auto& token : tokens) {
        std::cout << token << '\n';
      }
    }

    Parser parser(std::move(tokens));
    const auto expr = parser.parse();

    if (had_error) {
      return;
    }

    if (debug) {
      AstPrinter printer;
      std::cout << printer.print(*expr) << std::endl;
    }

    static Interpreter interpreter;
    interpreter.interpret(*expr);
  }


  void run_prompt(const bool debug)
  {
    while (true) {
      std::cout << "> ";
      std::string src;
      std::getline(std::cin, src);
      run(src, debug);
      had_error = false;
    }
  }


  void run_file(const std::string& path, const bool debug)
  {
    std::ifstream file(path);
    std::streamsize size = file.tellg();

    file.seekg(0, std::ios::beg);

    std::string src(static_cast<unsigned long>(size), '\0');
    if (!file.read(&src[0], size)) {
      throw std::ios_base::failure("Unable to read source file!");
    }

    run(src, debug);

    if (had_error) {
      std::exit(65);
    }
    else if (had_runtime_error) {
      std::exit(70);
    }
  }
}


int main(int argc, const char* argv[])
{
  ez::ezOptionParser opt;

  opt.overview = "Interpreter for the Lox language";
  opt.syntax = "loxx [OPTIONS] [script]";
  opt.add(
      "",
      false,
      0,
      0,
      "Enable debugging output.",
      "-d", "--debug"
  );

  opt.parse(argc, argv);

  const bool debug = opt.isSet("-d") != 0;

  if (opt.lastArgs.size() == 1) {
    loxx::run_file(*opt.lastArgs[0], debug);
  }
  else {
    loxx::run_prompt(debug);
  }
}