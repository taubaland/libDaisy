#include "daisy_boonta.h"

using namespace daisy;

#ifndef SAMPLE_RATE
// #define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#define SAMPLE_RATE 48014.f
#endif

// Hardware related defines.
// Switches
constexpr Pin TOG_SW_1_A_PIN = seed::D1;
constexpr Pin TOG_SW_1_B_PIN = seed::D2;
constexpr Pin TOG_SW_2_A_PIN = seed::D3;
constexpr Pin TOG_SW_2_B_PIN = seed::D4;
constexpr Pin TOG_SW_3_A_PIN = seed::D5;
constexpr Pin TOG_SW_3_B_PIN = seed::D6;
constexpr Pin SW_SEL_1_PIN = seed::D7;
constexpr Pin SW_SEL_2_PIN = seed::D8;
constexpr Pin SW_FS_1_PIN = seed::D9;
constexpr Pin SW_FS_2_PIN = seed::D10;

// Knobs
constexpr Pin PIN_EXPRESSION = seed::D15;
constexpr Pin PIN_KNOB_1     = seed::D16;
constexpr Pin PIN_KNOB_2     = seed::D17;
constexpr Pin PIN_KNOB_3     = seed::D18;
constexpr Pin PIN_KNOB_4     = seed::D19;
constexpr Pin PIN_KNOB_5     = seed::D20;
constexpr Pin PIN_KNOB_6     = seed::D21;

//Midi
constexpr Pin PIN_MIDI_OUT   = seed::D13;
constexpr Pin PIN_MIDI_IN    = seed::D14;

//Relay
constexpr Pin PIN_RELAY_OUT     = seed::D22;
constexpr Pin PIN_RELAY_BYPASS  = seed::D23;
constexpr Pin PIN_RELAY_IN      = seed::D24;

enum LedOrder
{
    LED_FOOTSWITCH_1_R,
    LED_FOOTSWITCH_1_G,
    LED_FOOTSWITCH_1_B,
    LED_SELECT_2_R,
    LED_SELECT_2_B,
    LED_SELECT_2_G,
    LED_SELECT_3_R,
    LED_SELECT_3_B,
    LED_SELECT_3_G,
    LED_SELECT_1_R,
    LED_SELECT_1_B,
    LED_SELECT_1_G,
    LED_FOOTSWITCH_2_R,
    LED_FOOTSWITCH_2_G,
    LED_FOOTSWITCH_2_B,
    LED_LAST,
};

static constexpr I2CHandle::Config i2c_config
  = {
        I2CHandle::Config::Peripheral::I2C_1,
        {Pin(PORTB, 8), Pin(PORTB, 9)},
        I2CHandle::Config::Speed::I2C_1MHZ
    };

static LedDriverPca9685<1, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    pedal_led_dma_buffer_a,
    pedal_led_dma_buffer_b;

void DaisyBoonta::Init(bool boost)
{
    // Set Some numbers up for accessors.
    // Initialize the hardware.
    seed.Configure();
    seed.Init(boost);
    InitI2C();
    InitLeds();
    InitSwitches();
    InitMidi();
    InitRelays();
    InitAnalogControls();
    SetAudioBlockSize(48);
    //seed.usb_handle.Init(UsbHandle::FS_INTERNAL);
}

void DaisyBoonta::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyBoonta::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].SetSampleRate(AudioCallbackRate());
    }
    expression.SetSampleRate(AudioCallbackRate());
}

void DaisyBoonta::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyBoonta::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyBoonta::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyBoonta::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyBoonta::StopAudio()
{
    seed.StopAudio();
}

void DaisyBoonta::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyBoonta::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisyBoonta::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyBoonta::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisyBoonta::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyBoonta::StartAdc()
{
    seed.adc.Start();
}

void DaisyBoonta::StopAdc()
{
    seed.adc.Stop();
}

void DaisyBoonta::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Process();
    }
    expression.Process();
}

float DaisyBoonta::GetKnobValue(Knob k)
{
    size_t idx;
    idx = k < KNOB_LAST ? k : KNOB_1;
    return knob[idx].Value();
}

float DaisyBoonta::GetExpression()
{
    return expression.Value();
}

void DaisyBoonta::ProcessDigitalControls()
{
    // Switch3 holds no state -- Read() is a pure function of its two pins, so
    // the toggles need no per-tick servicing. GetSwitchPosition() reads them
    // live.
    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Debounce();
    }
}

void DaisyBoonta::ClearLeds()
{
    for(size_t i = 0; i < FOOTSWITCH_LED_LAST; i++)
    {
        SetFootSwitchLed(static_cast<FootSwitchLed>(i), 0.0f, 0.0f, 0.0f);
    }
    for(size_t i = 0; i < SELECT_LED_LAST; i++)
    {
        SetSelectLed(static_cast<SelectLed>(i), 0.0f, 0.0f, 0.0f);
    }
}

void DaisyBoonta::UpdateLeds()
{
    led_driver_.SwapBuffersAndTransmit();
}

void DaisyBoonta::SetFootSwitchLed(FootSwitchLed idx, float r, float g, float b)
{
    uint8_t r_addr[FOOTSWITCH_LED_LAST] = { LED_FOOTSWITCH_1_R,
                                            LED_FOOTSWITCH_2_R};
    uint8_t g_addr[FOOTSWITCH_LED_LAST] = { LED_FOOTSWITCH_1_G,
                                            LED_FOOTSWITCH_2_G};
    uint8_t b_addr[FOOTSWITCH_LED_LAST] = { LED_FOOTSWITCH_1_B,
                                            LED_FOOTSWITCH_2_B};

    led_driver_.SetLed(r_addr[idx], r);
    led_driver_.SetLed(g_addr[idx], g);
    led_driver_.SetLed(b_addr[idx], b);
}

void DaisyBoonta::SetFootSwitchLed(FootSwitchLed idx, Color color)
{
    float r, g, b;
    r = color.Red();
    b = color.Blue();
    g = color.Green();
    SetFootSwitchLed(idx, r, g, b);
}

void DaisyBoonta::SetSelectLed(SelectLed idx, float r, float g, float b)
{
    uint8_t r_addr[SELECT_LED_LAST] = { LED_SELECT_2_R,
                                        LED_SELECT_1_R,
                                        LED_SELECT_3_R};
    uint8_t g_addr[SELECT_LED_LAST] = { LED_SELECT_2_B,
                                        LED_SELECT_1_B,
                                        LED_SELECT_3_B};
    uint8_t b_addr[SELECT_LED_LAST] = { LED_SELECT_2_G,
                                        LED_SELECT_1_G,
                                        LED_SELECT_3_G};

    led_driver_.SetLed(r_addr[idx], r);
    led_driver_.SetLed(g_addr[idx], g);
    led_driver_.SetLed(b_addr[idx], b);
}

void DaisyBoonta::SetSelectLed(SelectLed idx, Color color)
{
    float r, g, b;
    r = color.Red();
    b = color.Blue();
    g = color.Green();
    SetSelectLed(idx, r, g, b);
}

void DaisyBoonta::InitSwitches()
{
    // Two pins per toggle, in TogSw order.
    constexpr Pin pin_numbers[TOG_SW_LAST * 2] = {
        TOG_SW_1_A_PIN,
        TOG_SW_1_B_PIN,
        TOG_SW_2_A_PIN,
        TOG_SW_2_B_PIN,
        TOG_SW_3_A_PIN,
        TOG_SW_3_B_PIN,
    };

    constexpr Pin fsw_pin_numbers[SW_LAST] = {
        SW_SEL_2_PIN,
        SW_SEL_1_PIN,
        SW_FS_2_PIN,
        SW_FS_1_PIN,
    };

    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Init(fsw_pin_numbers[i]);
    }

    for(size_t i = 0; i < TOG_SW_LAST; i++)
    {
        toggle_switches[i].Init(pin_numbers[i * 2], pin_numbers[i * 2 + 1]);
    }
}

void DaisyBoonta::InitI2C()
{
    i2c_.Init(i2c_config);
}

void DaisyBoonta::InitLeds()
{
    // LEDs are on the LED Driver.
    uint8_t   addr[1] = {0x00};
    led_driver_.Init(i2c_, addr, pedal_led_dma_buffer_a, pedal_led_dma_buffer_b);
    ClearLeds();
    UpdateLeds();
}

void DaisyBoonta::InitAnalogControls()
{
    // Set order of ADCs based on CHANNEL NUMBER
    // KNOB_LAST + 1 because of Expression input
    AdcChannelConfig cfg[KNOB_LAST + 1];
    // Init with Single Pins
    cfg[KNOB_1].InitSingle(PIN_KNOB_6);
    cfg[KNOB_2].InitSingle(PIN_KNOB_5);
    cfg[KNOB_3].InitSingle(PIN_KNOB_4);
    cfg[KNOB_4].InitSingle(PIN_KNOB_3);
    cfg[KNOB_5].InitSingle(PIN_KNOB_2);
    cfg[KNOB_6].InitSingle(PIN_KNOB_1);
    // Special case for Expression
    cfg[KNOB_LAST].InitSingle(PIN_EXPRESSION);

    seed.adc.Init(cfg, KNOB_LAST + 1);
    // Make an array of pointers to the knob.
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate());
    }
    
    expression.Init(seed.adc.GetPtr(KNOB_LAST), AudioCallbackRate());
}

void DaisyBoonta::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyBoonta::InitRelays()
{
    // Relay Output
    relay_output.Init(PIN_RELAY_OUT, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL);

    // Relay Bypass
    relay_bypass.Init(PIN_RELAY_BYPASS, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL);
    
    // Relay Input
    relay_input.Init(PIN_RELAY_IN, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL);

}

void DaisyBoonta::SetRelay(GPIO relay, bool state)
{
    relay.Write(state);
}

void DaisyBoonta::ToggleRelay(GPIO relay)
{
    relay.Toggle();
}

int DaisyBoonta::GetSwitchPosition(TogSw s)
{
    size_t idx;
    idx = s < TOG_SW_LAST ? s : TOG_SW_1;
    return toggle_switches[idx].Read();
}

bool DaisyBoonta::GetButtonPressed(Sw s)
{
    size_t idx;
    idx = s < SW_LAST ? s : SW_SEL_1;
    return switches[idx].Pressed();
}

bool DaisyBoonta::CheckButtonLongPress(Sw s)
{
    size_t idx;
    idx = s < SW_LAST ? s : SW_SEL_1;
    return (switches[idx].TimeHeldMs() > 500);
}
