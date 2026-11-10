#include "cli/cmd.hpp"
#include "data/aurd.hpp"
#include <iostream>

namespace aura::cli {

int cmd_import(const Args& a) {
    if (a.positional.empty() || !a.has("o")) {
        std::cerr << "usage: aura import <in.csv> -o <out.aurd> [--compress]\n";
        return ARG_ERROR;
    }
    try {
        data::aurd::import_csv(a.positional[0], a.get("o"), a.has("compress"));
        return OK;
    } catch (const data::aurd::DatasetError& e) {
        std::cerr << "aura: data error: " << e.what() << "\n";
        return DATA_ERROR;
    }
}

} // namespace aura::cli