#include <fstream>
#include <iostream>
#include <string>

#include <ezOptionParser.hpp>

#include "AstPrinter.hpp"
#include "logging.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


namespace loxx
{
  struct DebugConfig
  {
    bool print_tokens;
    bool print_ast;
    bool print_bytecode;
    bool trace_exec;
  };


  void run(const std::string& src, const DebugConfig& debug_config,
           const bool in_repl)
  {
    Scanner scanner(src);
    auto tokens = scanner.scan_tokens();

    if (had_error) {
      return;
    }

    if (debug_config.print_tokens) {
      for (const auto& token : tokens) {
        std::cout << token << '\n';
      }
    }

    Parser parser(std::move(tokens), in_repl);
    const auto statements = parser.parse();

    if (had_error) {
      return;
    }

    if (debug_config.print_ast) {
      AstPrinter printer;
      std::cout << printer.print(statements) << std::endl;
    }

    static CodeObject compiler_output;
    static VirtualMachine vm(compiler_output, debug_config.trace_exec);

    Compiler compiler(vm, compiler_output);
    compiler.compile(statements);

    if (had_error) {
      return;
    }

    if (debug_config.print_bytecode) {
      print_bytecode(vm, compiler.output());
    }

    try {
      vm.execute();
    }
    catch (const RuntimeError& e) {
      runtime_error(e);
    }
  }


  void run_prompt(const DebugConfig& debug_config)
  {
    while (true) {
      std::cout << "> ";
      std::string src;
      std::getline(std::cin, src);
      run(src, debug_config, true);
      had_error = false;
    }
  }


  void run_file(const std::string& path, const DebugConfig& debug_config)
  {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();

    file.seekg(0, std::ios::beg);

    std::string src(static_cast<unsigned long>(size), '\0');
    if (!file.read(&src[0], size)) {
      throw std::ios_base::failure("Unable to read source file!");
    }

    run(src, debug_config, false);

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
      -1,
      ',',
      "Enable debugging output.",
      "-d", "--debug"
  );

  opt.parse(argc, argv);

  loxx::DebugConfig debug_config{false, false, false, false};

  if (opt.isSet("-d") != 0) {
    std::vector<std::string> selections;
    opt.get("-d")->getStrings(selections);
    for (const auto& s : selections) {
      debug_config.print_tokens   |= s == "tokens";
      debug_config.print_ast      |= s == "ast";
      debug_config.print_bytecode |= s == "bytecode";
      debug_config.trace_exec     |= s == "trace";
    }
  }

  if (opt.lastArgs.size() == 1) {
    loxx::run_file(*opt.lastArgs[0], debug_config);
  }
  else {
    loxx::run_prompt(debug_config);
  }
}
