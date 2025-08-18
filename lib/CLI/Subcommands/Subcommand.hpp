#ifndef CLI_SUBCOMMANDS_SUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_SUBCOMMAND_HPP

struct RunContext;

class ISubcommand {
   public:
    virtual ~ISubcommand() = default;

    /**
     * @brief Execute the subcommand
     * @param run_context Context for the run
     */
    virtual void execute(const RunContext &run_context) = 0;

   protected:
    const RunContext *m_run_context = nullptr;
};

#endif  // CLI_SUBCOMMANDS_SUBCOMMAND_HPP
