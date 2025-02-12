#pragma once
#ifndef DSY_PEDAL_125B_H
#define DSY_PEDAL_125B_H /**< & */

#include "daisy_seed.h"

namespace daisy
{
/**
   @brief Helpers and hardware definitions for custom daisy pedal boonta.
   @ingroup boards
*/
class DaisyBoonta
{
  public:
    /** Switches */
    enum TogSw
    {
        TOG_SW_1_A,   /**< Toggle */
        TOG_SW_1_B,   /**< Toggle */
        TOG_SW_2_A,   /**< Toggle */
        TOG_SW_2_B,   /**< Toggle */
        TOG_SW_3_A,   /**< Toggle */
        TOG_SW_3_B,   /**< Toggle */
        TOG_SW_LAST, /**< Last enum item */
    };

        enum Sw
    {
        SW_SEL_1,   /**< Footswitch */
        SW_SEL_2,   /**< Footswitch */
        SW_FS_1,    /**< Footswitch */
        SW_FS_2,    /**< Footswitch */
        SW_LAST, /**< Last enum item */
    };

    /** Knobs */
    enum Knob
    {
        KNOB_1,    /**< & */
        KNOB_2,    /**< & */
        KNOB_3,    /**< & */
        KNOB_4,    /**< & */
        KNOB_5,    /**< & */
        KNOB_6,    /**< & */
        KNOB_LAST, /**< & */
    };

    /** footswitch leds */
    enum FootSwitchLed
    {
        FOOTSWITCH_LED_1,    /**< & */
        FOOTSWITCH_LED_2,    /**< & */
        FOOTSWITCH_LED_LAST, /**< & */
    };

    /** select leds */
        enum SelectLed
    {
        SELECT_LED_1,        /**< & */
        SELECT_LED_2,        /**< & */
        SELECT_LED_3,        /**< & */
        SELECT_LED_LAST, /**< & */
    };

    /** Constructor */
    DaisyBoonta() {}
    /** Destructor */
    ~DaisyBoonta() {}

    /** Initialize daisy pedal */
    void Init(bool boost = true);

    /**
       Wait before moving on.
       \param del Delay time in ms.
     */
    void DelayMs(size_t del);

    /** Starts the callback
    \param cb Interleaved callback function
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Starts the callback
    \param cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running. */
    void StopAudio();

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number.
     */
    float AudioSampleRate();

    /** Sets the number of samples processed per channel by the audio callback.
       \param size Audio block size
     */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion. */
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();

    /** Call at the same frequency as controls are read for stable readings.*/
    void ProcessAnalogControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** Get value per knob.
    \param k Which knob to get
    \return Floating point knob position.
    */
    float GetKnobValue(Knob k);

    /** & */
    float GetExpression();

    /** Process digital controls */
    void ProcessDigitalControls();

    /** Turn all leds off */
    void ClearLeds();

    /** Update Leds to values you had set. */
    void UpdateLeds();

    /**
       Set FootSwitch LED colors
       \param idx Index to set
       \param r Red value
       \param g Green value
       \param b Blue value
    */
    void SetFootSwitchLed(FootSwitchLed idx, float r, float g, float b);

    /**
      Set Select LED colors
      \param idx Index to set
      \param r Red value
      \param g Green value
      \param b Blue value
    */
    void SetSelectLed(SelectLed idx, float r, float g, float b);

    DaisySeed seed;    /**< & */

    MidiUartHandler midi;                             /**< Handles midi*/

    AnalogControl knob[KNOB_LAST]; /**< & */
    AnalogControl expression;      /**< & */
    Switch        switches[SW_LAST]; /**< & */
    Switch3       toggle_switches[TOG_SW_LAST / 2]; /**< & */

    RgbLed    footswitch_led[FOOTSWITCH_LED_LAST]; /**< & */
    RgbLed    select_led[SELECT_LED_LAST]; /**< & */
    
    dsy_gpio relay_output; /**< &  */
    dsy_gpio relay_bypass; /**< &  */
    dsy_gpio relay_input;  /**< &  */

  private:
    void SetHidUpdateRates();
    void InitSwitches();
    void InitI2C();
    void InitLeds();
    void InitMidi();
    void ReadMidi();
    void InitRelays();
    void InitAnalogControls();
    void SetRelay(dsy_gpio, bool);

    inline uint16_t* adc_ptr(const uint8_t chn) { return seed.adc.GetPtr(chn); }

    // I2CHandle::Config i2c_config;
    I2CHandle i2c_;

    LedDriverPca9685<1, true> led_driver_;
};

} // namespace daisy

#endif
