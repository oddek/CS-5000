
#ifndef SUT_SINGLEWORKER_H
#define SUT_SINGLEWORKER_H

#include "RxPipeline.h"
#include "TxPipeline.h"

class SingleWorker
{
  public:
    SingleWorker(TxPipeline& txDriver_, RxPipeline& rxDriver_);
    bool update();


  private:
    TxPipeline& txDriver;
    RxPipeline& rxDriver;
};


#endif // SUT_SINGLEWORKER_H
