#include <fstream>
#include <iostream>
#include <string>

#include <args.hxx>

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


  Optional<DebugConfig> parse_debug_config(
      args::ValueFlagList<std::string>& opts)
  {
    DebugConfig ret{false, false, false, false};

    if (opts) {
      for (const auto& opt : args::get(opts)) {
        if (opt == "tokens") {
          ret.print_tokens = true;
        }
        else if (opt == "ast") {
          ret.print_ast = true;
        }
        else if (opt == "bytecode") {
          ret.print_bytecode = true;
        }
        else if (opt == "trace") {
          ret.trace_exec = true;
        }
        else {
          return {};
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
  args::ArgumentParser parser("Loxx - a C++ interpreter for the lox "
                              "programming language.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlagList<std::string> debug(
      parser,
      "debug",
      "Print debugging output (one of 'tokens', 'ast', 'bytecode' or 'trace'.",
      {'d', "debug"}
  );
  args::Positional<std::string> source_file(
      parser, "source file", "File containing source code to execute.");

  try {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Help&) {
    std::cout << parser;
    return EXIT_SUCCESS;
  }
  catch (const args::ParseError& e) {
    std::cerr << e.what() << '\n';
    std::cerr << parser;
    return EXIT_FAILURE;
  }
  catch (const args::ValidationError& e) {
    std::cerr << e.what() << '\n';
    std::cerr << parser;
    return EXIT_FAILURE;
  }

  const auto debug_config = loxx::parse_debug_config(debug);

  if (not debug_config) {
    std::cerr << "Invalid option to --debug flag.\n";
    std::cerr << parser;
    return EXIT_FAILURE;
  }

  try {
    if (source_file) {
      loxx::run_file(args::get(source_file), *debug_config);
    }
    else {
      loxx::run_prompt(*debug_config);
    }
  }
  catch (const std::ios_base::failure& e) {
    std::cerr << e.what() << '\n';
  }
  catch (const std::exception& e) {
    std::cerr << "Unhandled exception: " << e.what() << '\n';
  }
}
