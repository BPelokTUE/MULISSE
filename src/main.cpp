#include <print>
#include <string>

#include <CLI/CLI.hpp>

using namespace std;

int main(int argc, char** argv) {
    CLI::App app{"Run ULISSE-MTS"};

    // Add subcommands
    auto index = app.add_subcommand("index", "Construct ULISSE MTS index");
    auto search = app.add_subcommand("search", "Search using ULISSE MTS");
    app.require_subcommand(1);

    // Shared arguments
    string dataset_path;
    for (auto subcommand : {index, search}) {
        subcommand->add_option("-d,--dataset", dataset_path, "Dataset to use")->required()->check(CLI::ExistingFile);
    }

    // Indexation arguments
    string index_out_path;
    index->add_option("-o,--index_out", index_out_path, "Out path of the index file")->required();

    // Search arguments
    bool approximate_search = false;
    search->add_flag("-a,--approximate", approximate_search, "Do approximate search");

    // Execute command
    CLI11_PARSE(app, argc, argv);

    if (index->parsed()) {
        println("Preparing to index {}", dataset_path);
    } else if (search->parsed()) {
        println("Preparing to search on {}", dataset_path);
    }

    return 0;
}
