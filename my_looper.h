#include "daisy.h"
#include "daisysp.h"
#include "../../lib/daisy_dpt.h"

//#include "looper.h"

class MyLooper : public daisysp::Looper {
    public:
        // tell looper that we've received a "loop start" message
        //bool recording = true;
        bool length_set = false;
        bool length_setting = false;

        float started_at = 0.f;
        void LoopStart() {
            if (length_setting) {
                this->TrigRecord();
                recsize_ = pos_;
                length_setting = false;
                length_set = true;
            } else if (!length_set) {
                this->TrigRecord();
                this->TrigRecord();
                this->TrigRecord();
                this->TrigRecord();
                pos_ = 0.f;
                length_setting = true;
            } else {
                pos_ = 0.f;
            }
        }
        
};