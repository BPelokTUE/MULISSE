#ifndef SERIALIZATION_MACROS_HPP
#define SERIALIZATION_MACROS_HPP

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Enums/ArchiveType.hpp"
#include "Util/Types/Containers.hpp"

/**
 * @brief Macro to serializable / deserialize. Not intended to be used directly, but through MAKE_SERIALIZABLE.
 * @param archive_type Archive type, should be an ArchiveType enum value
 * @param stream Stream to serialize / deserialize to / from, should be an std::ofstream or std::ifstream
 * @param operation Operation to perform on the archive, should be a function that takes an archive as an argument
 * @param archive Archive type, should be either "InputArchive" or "OutputArchive"
 */
#define SERIALIZATION_MACRO(archive_type, stream, operation, archive) \
    switch (archive_type) {                                           \
        case JSON: {                                                  \
            cereal::JSON##archive archive(stream);                    \
            operation(archive);                                       \
            break;                                                    \
        }                                                             \
        case BINARY: {                                                \
            cereal::Binary##archive archive(stream);                  \
            operation(archive);                                       \
            break;                                                    \
        }                                                             \
        case NONE: {                                                  \
            break;                                                    \
        }                                                             \
            /* Add new archive types here */                          \
    }

/**
 * @brief Macro to make a class (de)serializable. Intended to be used in classes that inherit from
 * IFinalizedIndex.
 * @param members Members of the class to be serialized
 */
#define MAKE_SERIALIZABLE(members)                                                     \
   private:                                                                            \
    template <typename Archive>                                                        \
    void serialize(Archive &ar) {                                                      \
        ar members;                                                                    \
    }                                                                                  \
    template <typename Archive>                                                        \
    void deserialize(Archive &ar) {                                                    \
        ar members;                                                                    \
    }                                                                                  \
                                                                                       \
   public:                                                                             \
    void save(const str &out_file, ArchiveType ar_type) override {                     \
        std::ofstream ofs(add_archive_extension(out_file, ar_type), std::ios::binary); \
        SERIALIZATION_MACRO(ar_type, ofs, serialize, OutputArchive);                   \
    }                                                                                  \
    void load(const str &in_file, ArchiveType ar_type) override {                      \
        std::ifstream ifs(add_archive_extension(in_file, ar_type), std::ios::binary);  \
        if (!ifs.is_open()) throw std::runtime_error("Could not open index file");     \
        SERIALIZATION_MACRO(ar_type, ifs, deserialize, InputArchive);                  \
    }

#endif  // SERIALIZATION_MACROS_HPP
