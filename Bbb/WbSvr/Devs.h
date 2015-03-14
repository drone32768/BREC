#ifndef __DEVS__
#define __DEVS__

#include "../Util/mcf.h"
#include "../Util/net.h"
#include "../Util/gpioutil.h"

#include "../Adf4351/Adf4351.h"
#include "../Aboard/Aboard.h"
#include "../Hboard/Hboard.h"
#include "../Bboard/Bboard.h"
#include "../Rboard/Rboard.h"
#include "../Iboard/Iboard.h"
#include "../Mboard/Mboard.h"

class Devs {
  private:
    Adf4351      *mLo[2];
    AdcIf        *mAdc;
    Rboard       *mRbrd;
    Iboard       *mIbrd;

    int           OpenI();
  public:

    Devs();
    int      Open();
    AdcIf    *Adc()        { return(mAdc);    }
    Adf4351 *Lo( int idx ){ return(mLo[idx]);}
    Rboard  *Rbrd()       { return(mRbrd); }

};

Devs *Dp();

#endif
