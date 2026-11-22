#include "daisy_sunhaus.h"

using namespace daisy;

// Hardware related defines (Daisy Seed pin numbers).

// Encoders
constexpr Pin PIN_ENC_1_A    = seed::D20;
constexpr Pin PIN_ENC_1_B    = seed::D19;
constexpr Pin PIN_ENC_MENU_A = seed::D17;
constexpr Pin PIN_ENC_MENU_B = seed::D18;
constexpr Pin PIN_ENC_MENU_SW = seed::D10;

// Momentary switch (active low, internal pull-up)
constexpr Pin PIN_SW_MOMENTARY = seed::D13;

// Knobs
constexpr Pin PIN_KNOB_VOLUME      = seed::D15;
constexpr Pin PIN_KNOB_CLEAN_BLEND = seed::D16;

// LED
constexpr Pin PIN_LED = seed::D21;

void DaisySunhaus::QuadEncoder::Init(Pin a, Pin b)
{
    a_pin_.Init(a, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    b_pin_.Init(b, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    a_ = b_ = 0xff;
    inc_    = 0;
    last_update_ = System::GetNow();
}

// Same shift-register decode as daisy::Encoder, minus the switch.
void DaisySunhaus::QuadEncoder::Debounce()
{
    uint32_t now = System::GetNow();
    inc_         = 0;
    if(now - last_update_ < 1)
        return;
    last_update_ = now;

    a_ = (a_ << 1) | a_pin_.Read();
    b_ = (b_ << 1) | b_pin_.Read();
    if((a_ & 0x03) == 0x02 && (b_ & 0x03) == 0x00)
        inc_ = 1;
    else if((b_ & 0x03) == 0x02 && (a_ & 0x03) == 0x00)
        inc_ = -1;
}

void DaisySunhaus::Init(bool boost)
{
    seed.Configure();
    seed.Init(boost);
    InitEncoders();
    InitSwitches();
    InitLed();
    InitAnalogControls();
    SetAudioBlockSize(48);
}

void DaisySunhaus::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisySunhaus::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].SetSampleRate(AudioCallbackRate());
    }
}

void DaisySunhaus::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySunhaus::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisySunhaus::ChangeAudioCallback(
    AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySunhaus::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisySunhaus::StopAudio()
{
    seed.StopAudio();
}

void DaisySunhaus::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisySunhaus::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisySunhaus::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisySunhaus::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisySunhaus::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisySunhaus::StartAdc()
{
    seed.adc.Start();
}

void DaisySunhaus::StopAdc()
{
    seed.adc.Stop();
}

void DaisySunhaus::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Process();
    }
}

void DaisySunhaus::ProcessDigitalControls()
{
    enc_main.Debounce();
    enc_menu.Debounce();
    momentary.Debounce();
}

float DaisySunhaus::GetKnobValue(Knob k)
{
    size_t idx = k < KNOB_LAST ? k : KNOB_VOLUME;
    return knob[idx].Value();
}

int DaisySunhaus::GetEncoderIncrement(Enc e)
{
    return e == ENC_MENU ? enc_menu.Increment() : enc_main.Increment();
}

bool DaisySunhaus::MenuSwitchPressed()
{
    return enc_menu.Pressed();
}

bool DaisySunhaus::MenuSwitchRisingEdge()
{
    return enc_menu.RisingEdge();
}

bool DaisySunhaus::MomentaryPressed()
{
    return momentary.Pressed();
}

bool DaisySunhaus::MomentaryRisingEdge()
{
    return momentary.RisingEdge();
}

void DaisySunhaus::SetLed(bool on)
{
    led.Write(on);
}

void DaisySunhaus::InitEncoders()
{
    enc_main.Init(PIN_ENC_1_A, PIN_ENC_1_B);
    enc_menu.Init(PIN_ENC_MENU_A, PIN_ENC_MENU_B, PIN_ENC_MENU_SW);
}

void DaisySunhaus::InitSwitches()
{
    momentary.Init(PIN_SW_MOMENTARY,
                   0.f,
                   Switch::TYPE_MOMENTARY,
                   Switch::POLARITY_INVERTED,
                   GPIO::Pull::PULLUP);
}

void DaisySunhaus::InitLed()
{
    led.Init(PIN_LED, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL);
    led.Write(false);
}

void DaisySunhaus::InitAnalogControls()
{
    AdcChannelConfig cfg[KNOB_LAST];
    cfg[KNOB_VOLUME].InitSingle(PIN_KNOB_VOLUME);
    cfg[KNOB_CLEAN_BLEND].InitSingle(PIN_KNOB_CLEAN_BLEND);

    seed.adc.Init(cfg, KNOB_LAST);
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate());
    }
}
