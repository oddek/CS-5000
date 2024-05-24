
#ifndef UI_H
#define UI_H

namespace Ui
{
enum class Command
{
    None,
    Quit,
    Transmit,
    RxEnable,
    RxDisable,
    DataMoverSoftReset
};

Command getCommand();
void kill();
void init();
} // namespace Ui

#endif
