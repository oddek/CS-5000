
#include "Ui.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>
#include <signal.h>
#include <stdexcept>
#include <termios.h>

namespace Ui
{
static std::atomic<Command> LAST_RECEIVED_COMMAND{Command::None};
static pthread_t uiThread;
static bool threadContinue{true};

void got_signal(int sig)
{
    LAST_RECEIVED_COMMAND = Command::Quit;
}

Command getCommand()
{
    const Command cmd = LAST_RECEIVED_COMMAND;

    if (LAST_RECEIVED_COMMAND != Command::Quit)
    {
        LAST_RECEIVED_COMMAND = Command::None;
    }

    return cmd;
}

void* uiWorker(void* argument)
{
    (void)argument;

    static struct termios oldSettings;
    static struct termios newSettings;

    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(static_cast<uint32_t>(ICANON) | static_cast<uint32_t>(ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

    while (threadContinue)
    {
        const char input = getchar();

        if (input == 'd')
        {
            LAST_RECEIVED_COMMAND = Command::RxDisable;
        }

        else if (input == 'e')
        {
            LAST_RECEIVED_COMMAND = Command::RxEnable;
        }

        else if (input == 'w')
        {
            LAST_RECEIVED_COMMAND = Command::Transmit;
        }

        else if (input == 'r')
        {
            LAST_RECEIVED_COMMAND = Command::DataMoverSoftReset;
        }
        else if (input == 'q')
        {
            break;
        }
    }

    LAST_RECEIVED_COMMAND = Command::Quit;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    return nullptr;
}

void kill()
{
    threadContinue = false;
    pthread_kill(uiThread, SIGINT);
    pthread_join(uiThread, nullptr);
}

void init()
{
    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

    if (pthread_create(&uiThread, nullptr, uiWorker, nullptr) != 0)
    {
        throw std::runtime_error("UI: Create thread failed");
    }

    if (pthread_setname_np(uiThread, "RtTest_UiThread") != 0)
    {
        throw std::runtime_error("UI: Name thread failed");
    }
}
} // namespace Ui