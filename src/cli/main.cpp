#include "cli/cmd.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr <<
            "usage: aura <command> [args]\n\n"
            "commands:\n"
            "  import  <in.csv> -o <out.aurd> [--compress]\n"
            "  info    <data.aurd>\n"
            "  check   <strategy.aura>\n"
            "  run     <strategy.aura> --data <data.aurd> [--param name=value ...]\n"
            "          [--capital N] [--fee-bps N] [--slippage-bps N]\n"
            "          [--format json|text] [--interp] [--emit-ir]\n"
            "  sweep   <strategy.aura> --data <data.aurd>\n"
            "          --grid name=start:stop:step [--grid ...] [--threads N] [--sort-by sharpe]\n"
            "  serve   --data <data.aurd> [--port 8080]\n";
        return aura::cli::ARG_ERROR;
    }

    const std::string cmd = argv[1];
    aura::cli::Args args = aura::cli::parse_args(argc, argv, 2);

    if (cmd == "import") return aura::cli::cmd_import(args);
    if (cmd == "info")   return aura::cli::cmd_info  (args);
    if (cmd == "check")  return aura::cli::cmd_check (args);
    if (cmd == "run")    return aura::cli::cmd_run   (args);
    if (cmd == "sweep")  return aura::cli::cmd_sweep (args);
    if (cmd == "serve")  return aura::cli::cmd_serve (args);

    std::cerr << "aura: unknown command '" << cmd << "'\n";
    return aura::cli::ARG_ERROR;
}