//
//  ayb_kernel.cpp
//  openTSPS
//
//  Created by David Chanel on 8/7/13.
//
//

#include "ayb_kernel.h"


bool AYB_kernel::isOniSequence(string a_sFilename)
{
    vector <string> test = ofSplitString(a_sFilename, ".");
    if(test.back() == "oni")
        return true;
    else
        return false;
}