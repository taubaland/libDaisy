#pragma once
#ifndef DSY_PEDAL_SUNHAUS_H
#define DSY_PEDAL_SUNHAUS_H /**< & */

#include "daisy_seed.h"

namespace daisy
{
/**
   @brief Helpers and hardware definitions for the custom Sunhaus pedal.
   @ingroup boards

   Pins are Daisy Seed pin numbers (D<n>). Stereo audio in/out is on the
   Seed's codec.
*/
class DaisySunhaus
{
  public:
    /** Switchless quadrature encoder (libDaisy's Encoder requires a click pin). */
    class QuadEncoder
    {
      public:
        void Init(Pin a, Pin b);
        /** Call at ~1 kHz. */
        void Debounce();
        /** 
eturn -1, 0 or +1 for the last Debounce(). */
        int Increment() const { return inc_; }
        /** Raw pin levels (true = high), for wiring diagnostics. */
        bool RawA() { return a_pin_.Read(); }
        bool RawB() { return b_pin_.Read(); }

      private:
        GPIO     a_pin_, b_pin_;
        uint8_t  a_ = 0xff, b_ = 0xff;
        int      inc_ = 0;
        uint32_t last_update_ = 0;
    };

  public:
    /** Rotary encoders. ENC_1 has no switch; ENC_MENU has one. */
    enum Enc
    {
        ENC_1,    /**< Main encoder (no switch) */
        ENC_MENU, /**< Menu encoder with push switch */
        ENC_LAST, /**< Last enum item */
    };

    /** Knobs */
    enum Knob
    {
        KNOB_VOLUME,      /**< Output volume */
        KNOB_CLEAN_BLEND, /**< Clean / blend mix */
        KNOB_LAST,        /**< Last enum item */
    };

    /** Constructor */
    DaisySunhaus() {}
    /** Destructor */
    ~DaisySunhaus() {}

    /** Initialize the pedal */
    void Init(bool boost = true);

    /** Wait before moving on. \param del Delay time in ms. */
    void DelayMs(size_t del);

    /** Starts the callback \param cb Interleaved callback function */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Starts the callback \param cb multichannel callback function */
    void StartAudio(AudioHandle::AudioCallback cb);

    /** Switch callback functions \param cb New interleaved callback */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /** Switch callback functions \param cb New multichannel callback */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running. */
    void StopAudio();

    /** Updates the audio sample rate and reinitializes. Audio must be stopped. */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz. */
    float AudioSampleRate();

    /** Sets the number of samples per channel processed per callback. */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion. */
    void StartAdc();

    /** Stops transferring data from the ADC */
    void StopAdc();

    /** Call at the same frequency as controls are read for stable readings. */
    void ProcessAnalogControls();

    /** Debounce the encoders and the menu switch. Call at ~1 kHz. */
    void ProcessDigitalControls();

    /** Process analog and digital controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** \return Floating point knob position, 0..1. */
    float GetKnobValue(Knob k);

    /** \return Encoder increment since the last ProcessDigitalControls (-1/0/+1). */
    int GetEncoderIncrement(Enc e);

    /** \return true if the menu encoder switch is held. */
    bool MenuSwitchPressed();

    /** \return true on the tick the menu encoder switch goes down. */
    bool MenuSwitchRisingEdge();

    /** 
eturn true while the momentary switch (D13) is held. */
    bool MomentaryPressed();

    /** 
eturn true on the tick the momentary switch goes down. */
    bool MomentaryRisingEdge();

    /** Set the LED on or off. */
    void SetLed(bool on);

    DaisySeed seed; /**< & */

    AnalogControl knob[KNOB_LAST]; /**< & */
    QuadEncoder   enc_main;          /**< ENC_1 */
    Encoder       enc_menu;          /**< ENC_MENU, with switch */
    Switch        momentary;         /**< Momentary switch, pulled up */
    GPIO          led;               /**< Single-colour LED, on/off */

  private:
    void SetHidUpdateRates();
    void InitEncoders();
    void InitSwitches();
    void InitLed();
    void InitAnalogControls();
};

} // namespace daisy

#endif
