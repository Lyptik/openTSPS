Setup
------------
* copy openTSPS/addons/ofxOpenNI/{YOUR_OS}/copy_to_data_openni_path/lib to openTSPS/standalone/openNI/bin/data/openNI

==

Notes on how to get TSPS standalone to compile from a fresh start:

must compile in 32 bit not 64

hack xcode for 10.6 SDK if you want full quicktime support (not required)

must verify that the linked libs are all valid 
-Accelerate
-QTKit
-CoreVideo

make sure to add CoreVideo Library

websockets lib path is missing (copy from other projects)

