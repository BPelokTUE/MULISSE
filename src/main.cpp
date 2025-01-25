#include <print>
#include <string>
#include <fstream>

#include "CLI11/CLI11.hpp"
#include "Modules/RandomWalk.hpp"
#include "Modules/QueryGen.hpp"

using namespace std;

int main(int argc, char **argv) {
    CLI::App app{"Run ULISSE-MTS"};

    // Add subcommands
    auto create_ds = app.add_subcommand("create_ds", "Create random walk dataset");
    auto create_qs = app.add_subcommand("create_qs", "Create queries from dataset");
    auto index = app.add_subcommand("index", "Construct ULISSE MTS index");
    auto search = app.add_subcommand("search", "Search using ULISSE MTS");
    app.require_subcommand(1);

    // Define custom validators
    auto positive_int = CLI::Validator(
        [](std::string &input) {
            try {
                unsigned value = std::stoi(input);
                if (value > 0) {
                    return "";
                } else {
                    return "Value must be greater than 0";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "POSITIVE_INTEGER", "Positive Integer");

    // Add arguments
    /*
               | create_ds | create_query | index | search |
    dataset    |     X     |       X      |   X   |        |
    noise      |     X     |       X      |       |        |
    zero_start |     X     |              |       |        |
    n          |     X     |              |       |        |
    m          |     X     |       X      |   X   |        |
    c          |     X     |       X      |   X   |        |
    m_q        |           |       X      |       |        |
    q_noise    |           |       X      |       |        |
    lengths    |           |       X      |       |        |
    l_min      |           |              |   X   |        |
    l_max      |           |              |   X   |        |
    s          |           |              |   X   |        |
    n_start    |           |              |   X   |        |
    leaf_th    |           |              |   X   |        |
    query_path |           |              |   X   |   X    |
    approx/ex  |           |              |       |   X    |
    kNN/r-ran  |           |              |       |   X    |
    k(NN)      |           |              |       |   X    |
    r(range)   |           |              |       |   X    |
    normalize  |           |              |       |   X    |
    out        |           |              |       |   X    |
    */

    string dataset_path, query_path;
    create_ds->add_option("-d,--dataset", dataset_path, "Output dataset path")->required();
    for (auto subcommand : {create_qs, index}) {
        subcommand->add_option("-d,--dataset", dataset_path, "Dataset to use")->required()->check(CLI::ExistingFile);
    }
    create_qs->add_option("-q,--query", query_path, "Output query path")->required();
    search->add_option("-q,--query", query_path, "Query file")->required()->check(CLI::ExistingFile);

    float noise = 1.0;
    create_ds->add_option("--noise", noise, "Random walk standard deviation")->capture_default_str();
    create_qs->add_option("--noise", noise, "Query noise")->capture_default_str();

    bool zero_start = false;
    create_ds->add_flag("-z,--zero_start", zero_start, "Start the random walk from zero");

    unsigned num_series, series_len, num_channels;

    create_ds->add_option("-n,--num_series", num_series, "Number of series")->required()->check(positive_int);
    for (auto subcommand : {create_ds, create_qs, index}) {
        subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
        subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
            ->required()
            ->check(positive_int);
    }

    vec<unsigned> lengths;
    create_qs->add_option("-l,--lengths", lengths, "Query lengths")->required()->check(positive_int);

    unsigned num_queries;
    create_qs->add_option("--nq,--num_queries", num_queries, "Number of queries")->required()->check(positive_int);

    // Execute command
    CLI11_PARSE(app, argc, argv);

    if (create_ds->parsed()) {
        create_random_walks(dataset_path, noise, zero_start, num_series, series_len, num_channels);
    } else if (create_qs->parsed()) {
        create_queries(dataset_path, query_path, noise, series_len, num_channels, num_queries, lengths);
    } else if (index->parsed()) {
        println("Index creation not implemented");
    } else if (search->parsed()) {
        println("Search not implemented");
    }

    return 0;
}
