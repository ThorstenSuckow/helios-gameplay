module;


export module helios.gameplay.lifecycle.commands:LifetimeCommand;

export namespace helios::gameplay::lifecycle::commands {

    enum LifetimeCommandType {
        LifetimeExpired
    };

    template<typename THandle>
    struct LifetimeCommand {

        THandle handle;
        LifetimeCommandType type;
    };

}