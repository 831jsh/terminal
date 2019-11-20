// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "AppCommandline.h"

AppCommandline::AppCommandline()
{
    _BuildParser();
    _ResetStateToDefault();
}

void AppCommandline::_ResetStateToDefault()
{
    _profileName = "";
    _startingDirectory = "";
    _commandline.clear();
}

int AppCommandline::ParseCommand(const Cmdline& command)
{
    int localArgc = static_cast<int>(command.argc());
    auto localArgv = command.Argv();

    // std::cout << "######################### starting command #########################\n";
    // for (int i = 0; i < localArgc; i++)
    // {
    //     char* arg = localArgv[i];
    //     std::cout << "arg[" << i << "]=\"";
    //     printf("%s\"\n", arg);
    // }
    _ResetStateToDefault();

    try
    {
        if (localArgc == 2 && (std::string("/?") == localArgv[1] || std::string("-?") == localArgv[1]))
        {
            throw CLI::CallForHelp();
        }
        _app.clear();
        _app.parse(localArgc, localArgv);

        if (_NoCommandsProvided())
        {
            // std::cout << "Didn't find _any_ commands, using newTab to parse\n";
            _newTabCommand->clear();
            _newTabCommand->parse(localArgc, localArgv);
        }
    }
    catch (const CLI::CallForHelp& e)
    {
        return _app.exit(e);
    }
    catch (const CLI::ParseError& e)
    {
        if (_NoCommandsProvided())
        {
            // std::cout << "EXCEPTIONALLY Didn't find _any_ commands, using newTab to parse\n";
            try
            {
                _newTabCommand->clear();
                _newTabCommand->parse(localArgc, localArgv);
            }
            catch (const CLI::ParseError& e)
            {
                return _newTabCommand->exit(e);
            }
        }
        else
        {
            return _app.exit(e);
        }
    }
    return 0;
}

void AppCommandline::_BuildParser()
{
    // app{ "yeet, a test of the wt commandline" };

    ////////////////////////////////////////////////////////////////////////////
    _newTabCommand = _app.add_subcommand("new-tab", "Create a new tab");
    _newTabCommand->add_option("cmdline", _commandline, "Commandline to run in the given profile");
    _newTabCommand->add_option("-p,--profile", _profileName, "Open with the give profile");
    _newTabCommand->add_option("-d,--startingDirectory", _startingDirectory, "Open in the given directory instead of the profile's set startingDirectory");
    _newTabCommand->callback([&, this]() {
        std::cout << "######################### new-tab #########################\n";
        if (!_profileName.empty())
        {
            std::cout << "profileName: " << _profileName << std::endl;
        }
        else
        {
            std::cout << "Use the default profile" << std::endl;
        }
        if (!_startingDirectory.empty())
        {
            std::cout << "startingDirectory: " << _startingDirectory << std::endl;
        }
        else
        {
            std::cout << "Use the default startingDirectory" << std::endl;
        }
        if (_commandline.empty())
        {
            std::cout << "Use the default cmdline" << std::endl;
        }
        else
        {
            auto i = 0;
            for (auto arg : _commandline)
            {
                std::cout << "arg[" << i << "]=\"" << arg << "\"\n";
                i++;
            }
        }
    });
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    _listProfilesCommand = _app.add_subcommand("list-profiles", "List all the available profiles");
    ////////////////////////////////////////////////////////////////////////////
}

bool AppCommandline::_NoCommandsProvided()
{
    return !(*_listProfilesCommand || *_newTabCommand);
}

std::vector<Cmdline> AppCommandline::BuildCommands(int w_argc, wchar_t* w_argv[])
{
    std::wstring cmdSeperator = L";";
    std::vector<Cmdline> commands;
    commands.emplace_back(Cmdline{});
    for (auto i = 0; i < w_argc; i++)
    {
        const auto nextFullArg = std::wstring{ w_argv[i] };
        auto nextDelimiter = nextFullArg.find(cmdSeperator);
        if (nextDelimiter == std::wstring::npos)
        {
            commands.rbegin()->wargs.emplace_back(nextFullArg);
        }
        else
        {
            auto remaining = nextFullArg;
            auto nextArg = remaining.substr(0, nextDelimiter);
            remaining = remaining.substr(nextDelimiter + 1);
            commands.rbegin()->wargs.emplace_back(nextArg);
            do
            {
                // TODO: For delimiters that are escaped, skip them and go to the next
                nextDelimiter = remaining.find(cmdSeperator);
                commands.emplace_back(Cmdline{});
                commands.rbegin()->wargs.emplace_back(std::wstring{ L"wt.exe" });
                nextArg = remaining.substr(0, nextDelimiter);
                commands.rbegin()->wargs.emplace_back(nextArg);
                remaining = remaining.substr(nextDelimiter + 1);
            } while (nextDelimiter != std::wstring::npos);
        }
    }

    return commands;
}
