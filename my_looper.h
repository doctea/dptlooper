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
        float us_between_ticks;
        float us_between_ticks_when_recorded;

        void LoopStart() {
            tick_count = 0;
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
            tick_count++;
            static uint32_t last_clock_at = 0;
            //static uint32_t last_4_time = 0;
            static uint32_t start_4_count_at = daisy::System::GetUs();

            unsigned long now = daisy::System::GetUs();
            //us_between_ticks = now - last_clock_at;

            if (tick_count!=0 && tick_count%24==0) {
                us_between_ticks = (now - start_4_count_at) / 24.0;
                start_4_count_at = now;
                tick_count = 0;
            }

            /*if (tick_count%4==0) {
                //last_4_time += now;
                start_4_count_at = now;
            }*/

            last_clock_at = now;

            //if (tick_count==24) tick_count = 0;

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

        /** Get a floating point sample from the buffer */
        virtual  const float Read(size_t pos) override { 
            //return buff_[pos];
            float    a, b, frac;
            uint32_t i_idx = static_cast<uint32_t>(pos);
            frac           = pos - i_idx;
            a              = buff_[i_idx];
            b              = buff_[(i_idx + 1) % buffer_size_];
            return a + (b - a) * frac;
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
            return us_between_ticks_when_recorded / (float)us_between_ticks;

            /*float inc = 1.f;
            if(half_speed_)
                inc = 0.5f;
            return reverse_ ? -inc : inc;*/
        }
        

};