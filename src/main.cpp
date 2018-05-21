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


  DebugConfig parse_debug_config(ez::ezOptionParser& opt)
  {
    DebugConfig ret{false, false, false, false};

    if (opt.isSet("-d") != 0) {
      std::vector<std::string> selections;
      opt.get("-d")->getStrings(selections);

      if (selections.size() == 0) {
        throw std::runtime_error("No options supplied to --debug.");
      }

      for (const auto& s : selections) {
        if (s == "tokens") {
          ret.print_tokens = true;
        }
        else if (s == "ast") {
          ret.print_ast = true;
        }
        else if (s == "bytecode") {
          ret.print_bytecode = true;
        }
        else if (s == "trace") {
          ret.trace_exec = true;
        }
        else {
          throw std::runtime_error("Unknown argument to --debug: " + s);
        }
      }
    }
    return ret;
  }


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

    static std::vector<CodeObject> code_objects{CodeObject{}};
    code_objects.front().bytecode.reserve(10240);
    static VirtualMachine vm(debug_config.trace_exec);

    Compiler compiler(vm, debug_config.print_bytecode);
    compiler.compile(statements);

    if (had_error) {
      return;
    }

    if (debug_config.print_bytecode) {
      print_bytecode(vm, "top level", compiler.output());
    }

    try {
      vm.execute(compiler.output());
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
    if (not file.good()) {
      throw std::ios_base::failure("Unable to open source file!");
    }

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

  try {
    const auto debug_config = loxx::parse_debug_config(opt);
    
    if (opt.lastArgs.size() == 1) {
      loxx::run_file(*opt.lastArgs[0], debug_config);
    }
    else {
      loxx::run_prompt(debug_config);
    }
  }
  catch (const std::runtime_error& e) {
    std::cerr << "Runtime error: " << e.what() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Unhandled exception: " << e.what() << '\n';
  }
}
