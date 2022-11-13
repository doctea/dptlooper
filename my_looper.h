#include "daisy.h"
#include "daisysp.h"
#include "../../lib/daisy_dpt.h"

#include "looper.h"

class MyLooper : public daisysp_custom::Looper {
    public:
        // tell looper that we've received a "loop start" message
        //bool recording = true;
        bool length_set = false;
        bool length_setting = false;

        /*float started_at = 0.f;
        float duration; // length of clip in seconds
        float speed = 1.f;*/
        float beats_per_minute;

        uint32_t tick_count = 0;
        uint32_t us_between_ticks;
        float us_between_ticks_when_recorded;
        float us_between_ticks_now;

        void LoopStart() {
            if (length_setting) {
                this->TrigRecord();
                //duration = recsize_ / 48000;
                //recsize_ = pos_;
                length_setting = false;
                length_set = true;
                us_between_ticks_when_recorded = us_between_ticks;
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
            tick_count = 0;
        }

        void ReceiveTick() {
            static uint32_t last_clock_at = 0;

            unsigned long now = daisy::System::GetUs();
            us_between_ticks = now - last_clock_at;
            //last_clock_at = now;

            // 60,000 (ms) ÷ BPM = duration of a quarter note
            /*float beat_duration = us_between_ticks * 24.0; // length of 1 beat in microseconds
            float beats_per_second = 1000.0 / beat_duration;
            beats_per_minute = beats_per_second * 60.0;*/
        }

        void SetState(Looper::State state) {
            state_ = state;
        }
        void SetPlaying() {
            SetState(Looper::State::PLAYING);
        }
        void SetDubbing() {
            SetState(Looper::State::REC_DUB);
        }

        /*void SetSpeed(float speed) {
            this->speed = speed;
        }
        unsigned long time_ticked = 0;
        void SetTimeTick(unsigned long ms_per_tick) {
            time_ticked += ms_per_tick;
        }*/


    protected:
        /** Private Member Functions */
        virtual float GetIncrementSize() override {
            //float samples_per_ppqn = (48000 * 60) / (24*4*4);
            //float num_ticks_per_loop = 24 * 4;
            
            // speed = distance/time;
            //return this->speed;
            return us_between_ticks_when_recorded / us_between_ticks;

            /*float inc = 1.f;
            if(half_speed_)
                inc = 0.5f;
            return reverse_ ? -inc : inc;*/
        }
        
};