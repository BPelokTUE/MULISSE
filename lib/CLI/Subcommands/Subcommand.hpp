#ifndef CLI_SUBCOMMANDS_SUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_SUBCOMMAND_HPP

struct CommonOptions;

class ISubcommand {
   public:
    virtual ~ISubcommand() = default;

    /**
     * @brief Set up execution after parsing arguments
     * @param common_opts Common options for all subcommands
     * */
    virtual void set_up_execution(const CommonOptions *common_opts);

    /** @brief Do additional argument validation after setting up run */
    virtual void validate_arguments();

    /** @brief Execute the subcommand */
    virtual void execute() = 0;

   protected:
    const CommonOptions *m_common_opts = nullptr;
};

#endif  // CLI_SUBCOMMANDS_SUBCOMMAND_HPP
