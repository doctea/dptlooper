#include "daisy.h"
#include "daisysp.h"
#include "../../lib/daisy_dpt.h"

#include "looper.h"
#include "my_looper.h"

/*
    Hello, friend. Here is a minimal, normal DPT template w/ a few notes / examples.

    We hope you enjoy your time with DPT and Daisy Patch SM!

    ~ j.m.
*/

#define kBuffSize 48000 * 60 // 60 seconds at 48kHz

using namespace daisy;
using namespace daisysp_custom;
using namespace daisysp;
using namespace dpt;

DPT patch;

MyLooper              looper_l;
MyLooper              looper_r;
float DSY_SDRAM_BSS buffer_l[kBuffSize];
float DSY_SDRAM_BSS buffer_r[kBuffSize];

float cv6_state = 0.0f;

void dac7554callback(void *data) {
    /*
        Update CV 3-6 here, like this (voltages -7.0 to 7.0)
            patch.WriteCvOutExp(
                1.0,
                2.0,
                3.0,
                4.0,
                false);
            );

            or for raw values, 
            patch.WriteCvOutExp(
                0,
                1023,
                2047
                4095,
                true);
            );
        */
}

void AudioCallback(AudioHandle::InputBuffer in,
                       AudioHandle::OutputBuffer out,
                       size_t size)
{
    patch.ProcessAllControls();

    float in_level   = patch.controls[CV_1].Value();
    float loop_level = patch.controls[CV_2].Value();

    static bool last_state = false;
    if(!last_state && patch.controls[CV_3].Value() >= 0.5) {
        looper_l.TrigRecord();
        looper_r.TrigRecord();
    } else if (last_state && patch.controls[CV_3].Value() < 0.5) {
        looper_l.TrigRecord();
        looper_r.TrigRecord();
    }
    last_state = patch.controls[CV_3].Value() >= 0.5;

    if (patch.controls[CV_4].Value() > 0.5) {
        looper_l.Clear();
        looper_r.Clear();
    }

    if (looper_l.length_setting) {
        patch.WriteCvOut(3, -5.0, false);    
    } else if (looper_l.length_set) {
        patch.WriteCvOut(3, 5.0, false);
    } else {
        patch.WriteCvOut(3, 0.0, false);
    }

    patch.WriteCvOut(CV_OUT_1, last_state ? -5.0 : 10.0, false);
    //patch.WriteCvOut(2, last_state ? -5.0 : 10.0, false);
    patch.WriteCvOut(CV_OUT_2, cv6_state, false);

    //float speed_knob = patch.controls[CV_8].Value();
    //looper_l.SetSpeed(speed_knob);
    //looper_r.SetSpeed(speed_knob);
 
    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        // store the inputs * the input gain factor
        float in_l = IN_L[i] * in_level;
        float in_r = IN_R[i] * in_level;

        // store signal = loop signal * loop gain + in * in_gain
        float sig_l = looper_l.Process(in_l) * loop_level + in_l;
        float sig_r = looper_r.Process(in_r) * loop_level + in_r;

        // send that signal to the outputs
        OUT_L[i] = sig_l;
        OUT_R[i] = sig_r;
    }
    /*
        CV 1 - 8 accessed
            patch.controls[CV_1].Value()

        Gate ins acccessed

            patch.gate_in_1.State();
            patch.gate_in_2.State();

        Send data to gate outs

            dsy_gpio_write(&patch.gate_out_1, 1);
            dsy_gpio_write(&patch.gate_out_2, 2);

        Send data to CV 1 and 2 (0 is both, currently it seems 1 is CV2, 2 is CV1)
            patch.WriteCvOut(0, 5.0, false);
            patch.WriteCvOut(0, [0 - 4095], true); // last argument is 'raw', send raw 12-bit data
        
    */
    /*for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }*/
}

int main(void)
{
    patch.Init();

    // Set up callback for TIM5

    patch.StartAudio(AudioCallback);
    patch.InitTimer(dac7554callback, nullptr);

    // Init the loopers
    looper_l.Init(buffer_l, kBuffSize);
    looper_r.Init(buffer_r, kBuffSize);

    looper_l.SetMode(daisysp_custom::Looper::Mode::FRIPPERTRONICS);
    looper_r.SetMode(daisysp_custom::Looper::Mode::FRIPPERTRONICS);


    patch.midi.StartReceive();

    // Start the audio callback
    patch.StartAudio(AudioCallback);

    static unsigned long last_clock = daisy::System::GetUs();
    static unsigned long us_between_ticks = 2033;

    while(1)
    {
        /*
            MIDI processing here

            patch.midi.Listen()
            first then you can call

            patch.midi.HasEvents()
            and
            auto event = patch.midi.PopEvent();

            and sort that out from there

            if(event.type == MidiMessageType::NoteOn) {
                ...
            };
        */
        patch.midi.Listen();

        while(patch.midi.HasEvents()) {
            cv6_state = 5.0f;
            //patch.WriteCvOut(CV_6, 1.0, false);
            auto event = patch.midi.PopEvent();
            if (event.type==MidiMessageType::SystemRealTime) {
                dsy_gpio_write(&patch.gate_out_2, 1);
                if (event.srt_type==SystemRealTimeType::TimingClock) {
                    looper_l.ReceiveTick();
                    looper_r.ReceiveTick();
                } else if(event.srt_type==SystemRealTimeType::Start) {
                    //patch.WriteCvOut(CV_6, 5.0, false);
                    looper_l.LoopStart();
                    looper_r.LoopStart();
                } else if (event.srt_type==SystemRealTimeType::Stop) {
                    looper_l.SetPlaying();
                } else if (event.srt_type==SystemRealTimeType::Continue) {
                    looper_l.SetDubbing();
                }
            } else if(event.type  == MidiMessageType::NoteOn) {
                dsy_gpio_write(&patch.gate_out_1, 1);
            } else if(event.type  == MidiMessageType::NoteOff) {
                dsy_gpio_write(&patch.gate_out_1, 0);
            } 
        }
        dsy_gpio_write(&patch.gate_out_2, 0);
        cv6_state = -5.0f;
    }
}