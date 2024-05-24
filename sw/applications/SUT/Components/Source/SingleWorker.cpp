
#include "SingleWorker.h"


SingleWorker::SingleWorker(TxPipeline& txDriver_, RxPipeline& rxDriver_) : txDriver(txDriver_), rxDriver(rxDriver_)
{
}

bool SingleWorker::update()
{
    bool runMore = txDriver.update();
    runMore |= rxDriver.update();
    return runMore;
}
