//--------------------------------------------------------------------------------------
// File: Audio.h
//
// DirectXTK for Audio header
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#include <objbase.h>
#include <mmreg.h>
#include <audioclient.h>

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <xma2defs.h>
#pragma comment(lib,"acphal.lib")
#endif

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#if defined(_MSC_VER) && (_MSC_VER < 1700)
#error DirectX Tool Kit for Audio does not support VS 2010 without the DirectX SDK 
#endif
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib")
#else
// Using XAudio 2.7 requires the DirectX SDK
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\comdecl.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\xaudio2.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\xaudio2fx.h>
#pragma warning(push)
#pragma warning( disable : 4005 )
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\x3daudio.h>
#pragma warning(pop)
#pragma comment(lib,"x3daudio.lib")
#endif

#include <DirectXMath.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#pragma warning(pop)

#include <memory>
#include <string>
#include <vector>

#pragma warning(push)
#pragma warning(disable : 4481)
// VS 2010 considers 'override' to be a extension, but it's part of C++11 as of VS 2012

namespace DirectX
{
    #if (DIRECTXMATH_VERSION < 305) && !defined(XM_CALLCONV)
    #define XM_CALLCONV __fastcall
    typedef const XMVECTOR& HXMVECTOR;
    typedef const XMMATRIX& FXMMATRIX;
    #endif

    class SoundEffectInstance;

    //----------------------------------------------------------------------------------
    struct AudioStatistics
    {
        size_t  playingOneShots;        // Number of one-shot sounds currently playing
        size_t  playingInstances;       // Number of sound effect instances currently playing
        size_t  allocatedInstances;     // Number of SoundEffectInstance allocated
        size_t  allocatedVoices;        // Number of XAudio2 voices allocated (standard, 3D, and one-shots) 
        size_t  allocatedVoices3d;      // Number of XAudio2 voices allocated for 3D
        size_t  allocatedVoicesOneShot; // Number of XAudio2 voices allocated for one-shot sounds
        size_t  audioBytes;             // Total wave data (in bytes) in SoundEffects and in-memory WaveBanks
#if defined(_XBOX_ONE) && defined(_TITLE)
        size_t  xmaAudioBytes;          // Total wave data (in bytes) in SoundEffects and in-memory WaveBanks allocated with ApuAlloc
#endif
    };


    //----------------------------------------------------------------------------------
    class IVoiceNotify
    {
    public:
        virtual void OnBufferEnd() = 0;
            // Notfication that a voice buffer has finished
            // Note this is called from XAudio2's worker thread, so it should perform very minimal and thread-safe operations

        virtual void OnCriticalError() = 0;
            // Notification that the audio engine encountered a critical error

        virtual void OnReset() = 0;
            // Notification of an audio engine reset

        virtual void OnDestroyEngine() = 0;
            // Notification that the audio engine is being destroyed

        virtual void GatherStatistics( AudioStatistics& stats ) const = 0;
            // Contribute to statistics request
    };

    //----------------------------------------------------------------------------------
    enum AUDIO_ENGINE_FLAGS
    {
        AudioEngine_Default             = 0x0,

        AudioEngine_EnvironmentalReverb = 0x1,
        AudioEngine_ReverbUseFilters    = 0x2,

        AudioEngine_Debug               = 0x10000,
        AudioEngine_ThrowOnNoAudioHW    = 0x20000,
    };

    inline AUDIO_ENGINE_FLAGS operator|(AUDIO_ENGINE_FLAGS a, AUDIO_ENGINE_FLAGS b) { return static_cast<AUDIO_ENGINE_FLAGS>( static_cast<int>(a) | static_cast<int>(b) ); }

    enum SOUND_EFFECT_INSTANCE_FLAGS
    {
        SoundEffectInstance_Default             = 0x0,

        SoundEffectInstance_Use3D               = 0x1,
        SoundEffectInstance_ReverbUseFilters    = 0x2,

        SoundEffectInstance_UseRedirectLFE      = 0x10000,
    };

    inline SOUND_EFFECT_INSTANCE_FLAGS operator|(SOUND_EFFECT_INSTANCE_FLAGS a, SOUND_EFFECT_INSTANCE_FLAGS b) { return static_cast<SOUND_EFFECT_INSTANCE_FLAGS>( static_cast<int>(a) | static_cast<int>(b) ); }

    enum AUDIO_ENGINE_REVERB
    {
        Reverb_Off,
        Reverb_Default,
        Reverb_Generic,
        Reverb_Forest,
        Reverb_PaddedCell,
        Reverb_Room,
        Reverb_Bathroom,
        Reverb_LivingRoom,
        Reverb_StoneRoom,
        Reverb_Auditorium,
        Reverb_ConcertHall,
        Reverb_Cave,
        Reverb_Arena,
        Reverb_Hangar,
        Reverb_CarpetedHallway,
        Reverb_Hallway,
        Reverb_StoneCorridor,
        Reverb_Alley,
        Reverb_City,
        Reverb_Mountains,
        Reverb_Quarry,
        Reverb_Plain,
        Reverb_ParkingLot,
        Reverb_SewerPipe,
        Reverb_Underwater,
        Reverb_SmallRoom,
        Reverb_MediumRoom,
        Reverb_LargeRoom,
        Reverb_MediumHall,
        Reverb_LargeHall,
        Reverb_Plate,
        Reverb_MAX
    };


    //----------------------------------------------------------------------------------
    class AudioEngine
    {
    public:
        explicit AudioEngine( AUDIO_ENGINE_FLAGS flags = AudioEngine_Default, _In_opt_ const WAVEFORMATEX* wfx = nullptr, _In_opt_z_ const wchar_t* deviceId = nullptr, 
                              AUDIO_STREAM_CATEGORY category = AudioCategory_GameEffects );

        AudioEngine(AudioEngine&& moveFrom);
        AudioEngine& operator= (AudioEngine&& moveFrom);
        virtual ~AudioEngine();

        bool Update();
            // Performs per-frame processing for the audio engine, returns false if in 'silent mode'

        bool Reset( _In_opt_ const WAVEFORMATEX* wfx = nullptr, _In_opt_z_ const wchar_t* deviceId = nullptr );
            // Reset audio engine from critical error/silent mode using a new device; can also 'migrate' the graph
            // Returns true if succesfully reset, false if in 'silent mode' due to no default device
            // Note: One shots are lost, all SoundEffectInstances are in the STOPPED state after successful reset

        void Suspend();
        void Resume();
            // Suspend/resumes audio processing (i.e. global pause/resume)

        void SetReverb( AUDIO_ENGINE_REVERB reverb );
        void SetReverb( _In_opt_ const XAUDIO2FX_REVERB_PARAMETERS* native );
            // Sets environmental reverb for 3D positional audio (if active)

        AudioStatistics GetStatistics() const;
            // Gathers audio engine statistics

        WAVEFORMATEXTENSIBLE GetOutputFormat() const;
            // Returns the format consumed by the mastering voice (which is the same as the device output if defaults are used)

        uint32_t GetChannelMask() const;
            // Returns the output channel mask

        uint32_t GetOutputChannels() const;
            // Returns the number of output channels

        bool IsAudioDevicePresent() const;
            // Returns true if the audio graph is operating normally, false if in 'silent mode'

        bool IsCriticalError() const;
            // Returns true if the audio graph is halted due to a critical error (which also places the engine into 'silent mode')

        // Voice pool management.
        void AllocateVoice( _In_ const WAVEFORMATEX* wfx, SOUND_EFFECT_INSTANCE_FLAGS flags, bool oneshot, _Outptr_ IXAudio2SourceVoice** voice );

        void RegisterNotify( _In_ IVoiceNotify* notify );
        void UnregisterNotify( _In_ IVoiceNotify* notify, bool usesOneShots );

        // XAudio2 interface access
        IXAudio2* GetInterface() const;
        IXAudio2MasteringVoice* GetMasterVoice() const;
        IXAudio2SubmixVoice* GetReverbVoice() const;
        X3DAUDIO_HANDLE& Get3DHandle() const;

        struct RendererDetail
        {
            std::wstring deviceId;
            std::wstring description;
        };

        static std::vector<RendererDetail> GetRendererDetails();
            // Returns a list of valid audio endpoint devices

    private:
        // Private implementation.
        class Impl;
        std::unique_ptr<Impl> pImpl;

        // Prevent copying.
        AudioEngine(AudioEngine const&);
        AudioEngine& operator= (AudioEngine const&);
    };


    //----------------------------------------------------------------------------------
    class WaveBank
    {
    public:
        WaveBank( _In_ AudioEngine* engine, _In_z_ const wchar_t* wbFileName );

        WaveBank(WaveBank&& moveFrom);
        WaveBank& operator= (WaveBank&& moveFrom);
        virtual ~WaveBank();

        void Play( uint32_t index );
        void Play( _In_z_ const char* name );

        std::unique_ptr<SoundEffectInstance> CreateInstance( uint32_t index, SOUND_EFFECT_INSTANCE_FLAGS flags = SoundEffectInstance_Default );
        std::unique_ptr<SoundEffectInstance> CreateInstance( _In_z_ const char* name, SOUND_EFFECT_INSTANCE_FLAGS flags = SoundEffectInstance_Default );

        bool IsPrepared() const;
        bool IsInUse() const;
        bool IsStreamingBank() const;

        uint32_t SampleSizeInBytes( uint32_t index ) const;

        uint32_t SampleDuration( uint32_t index ) const;
            // Returns the duration in samples

        const WAVEFORMATEX* GetFormat( uint32_t index, _Out_writes_bytes_(maxsize) WAVEFORMATEX* wfx, size_t maxsize ) const;

        uint32_t Find( _In_z_ const char* name ) const;

#if defined(_XBOX_ONE) || (_WIN32_WINNT < _WIN32_WINNT_WIN8)
        bool FillSubmitBuffer( uint32_t index, _Out_ XAUDIO2_BUFFER& buffer, _Out_ XAUDIO2_BUFFER_WMA& wmaBuffer ) const;
#else
        void FillSubmitBuffer( uint32_t index, _Out_ XAUDIO2_BUFFER& buffer ) const;
#endif

    private:
        // Private implementation.
        class Impl;

        std::unique_ptr<Impl> pImpl;

        // Prevent copying.
        WaveBank(WaveBank const&);
        WaveBank& operator= (WaveBank const&);

        // Private interface
        void UnregisterInstance( _In_ SoundEffectInstance* instance );

        friend class SoundEffectInstance;
    };


    //----------------------------------------------------------------------------------
    class SoundEffect
    {
    public:
        SoundEffect( _In_ AudioEngine* engine, _In_z_ const wchar_t* waveFileName );

        SoundEffect( _In_ AudioEngine* engine, _Inout_ std::unique_ptr<uint8_t[]>& wavData,
                     _In_ const WAVEFORMATEX* wfx, _In_reads_bytes_(audioBytes) const uint8_t* startAudio, uint32_t audioBytes );

        SoundEffect( _In_ AudioEngine* engine, _Inout_ std::unique_ptr<uint8_t[]>& wavData,
                     _In_ const WAVEFORMATEX* wfx, _In_reads_bytes_(audioBytes) const uint8_t* startAudio, uint32_t audioBytes,
                     uint32_t loopStart, uint32_t loopLength );

#if defined(_XBOX_ONE) || (_WIN32_WINNT < _WIN32_WINNT_WIN8)

        SoundEffect( _In_ AudioEngine* engine, _Inout_ std::unique_ptr<uint8_t[]>& wavData,
                     _In_ const WAVEFORMATEX* wfx, _In_reads_bytes_(audioBytes) const uint8_t* startAudio, uint32_t audioBytes,
                     _In_reads_(seekCount) const uint32_t* seekTable, uint32_t seekCount );

#endif

        SoundEffect(SoundEffect&& moveFrom);
        SoundEffect& operator= (SoundEffect&& moveFrom);
        virtual ~SoundEffect();

        void Play();

        std::unique_ptr<SoundEffectInstance> CreateInstance( SOUND_EFFECT_INSTANCE_FLAGS flags = SoundEffectInstance_Default );

        bool IsInUse() const;

        uint32_t SampleSizeInBytes() const;

        uint32_t SampleDuration() const;
            // Returns the duration in samples

        const WAVEFORMATEX* GetFormat() const;

#if defined(_XBOX_ONE) || (_WIN32_WINNT < _WIN32_WINNT_WIN8)
        bool FillSubmitBuffer( _Out_ XAUDIO2_BUFFER& buffer, _Out_ XAUDIO2_BUFFER_WMA& wmaBuffer ) const;
#else
        void FillSubmitBuffer( _Out_ XAUDIO2_BUFFER& buffer ) const;
#endif

    private:
        // Private implementation.
        class Impl;

        std::unique_ptr<Impl> pImpl;

        // Prevent copying.
        SoundEffect(SoundEffect const&);
        SoundEffect& operator= (SoundEffect const&);

        // Private interface
        void UnregisterInstance( _In_ SoundEffectInstance* instance );

        friend class SoundEffectInstance;
    };


    //----------------------------------------------------------------------------------
    struct AudioListener : public X3DAUDIO_LISTENER
    {
        AudioListener()
        {
            memset( this, 0, sizeof(X3DAUDIO_LISTENER) );

            OrientFront.z =
            OrientTop.y = 1.f;
        }

        void XM_CALLCONV SetPosition( FXMVECTOR v )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Position ), v );
        }
        void SetPosition( const XMFLOAT3& pos )
        {
            Position.x = pos.x;
            Position.y = pos.y;
            Position.z = pos.z;
        }

        void XM_CALLCONV SetVelocity( FXMVECTOR v )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Velocity ), v );
        }
        void SetVelocity( const XMFLOAT3& vel )
        {
            Velocity.x = vel.x;
            Velocity.y = vel.y;
            Velocity.z = vel.z;
        }

        void XM_CALLCONV SetOrientation( FXMVECTOR forward, FXMVECTOR up )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), forward );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), up );
        }
        void SetOrientation( const XMFLOAT3& forward, const XMFLOAT3& up )
        {
            OrientFront.x = forward.x;  OrientTop.x = up.x;
            OrientFront.y = forward.y;  OrientTop.y = up.y;
            OrientFront.z = forward.z;  OrientTop.z = up.z;
        }

        void XM_CALLCONV SetOrientationFromQuaternion( FXMVECTOR quat )
        {
            XMVECTOR forward = XMVector3Rotate( g_XMIdentityR2, quat );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), forward );

            XMVECTOR up  = XMVector3Rotate( g_XMIdentityR1, quat );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), up );
        }

        void XM_CALLCONV Update( FXMVECTOR newPos, XMVECTOR upDir, float dt )
            // Updates velocity and orientation by tracking changes in position over time...
        {
            if ( dt > 0.f )
            {
                XMVECTOR lastPos = XMLoadFloat3( reinterpret_cast<const XMFLOAT3*>( &Position ) );

                XMVECTOR vDelta = ( newPos - lastPos );
                XMVECTOR v = vDelta / dt;
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Velocity ), v );

                vDelta = XMVector3Normalize( vDelta );
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), vDelta );

                v = XMVector3Cross( upDir, vDelta );
                v = XMVector3Normalize( v );

                v = XMVector3Cross( vDelta, v );
                v = XMVector3Normalize( v );
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), v );

                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Position ), newPos );
            }
        }
    };


    //----------------------------------------------------------------------------------
    struct AudioEmitter : public X3DAUDIO_EMITTER
    {
        float       EmitterAzimuths[XAUDIO2_MAX_AUDIO_CHANNELS];

        AudioEmitter()
        {
            memset( this, 0, sizeof(X3DAUDIO_EMITTER) );
            memset( EmitterAzimuths, 0, sizeof(EmitterAzimuths) );

            OrientFront.z =
            OrientTop.y =
            ChannelRadius = 
            CurveDistanceScaler =
            DopplerScaler = 1.f;

            ChannelCount = 1;
            pChannelAzimuths = EmitterAzimuths;

            InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
        }

        void XM_CALLCONV SetPosition( FXMVECTOR v )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Position ), v );
        }
        void SetPosition( const XMFLOAT3& pos )
        {
            Position.x = pos.x;
            Position.y = pos.y;
            Position.z = pos.z;
        }

        void XM_CALLCONV SetVelocity( FXMVECTOR v )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Velocity ), v );
        }
        void SetVelocity( const XMFLOAT3& vel )
        {
            Velocity.x = vel.x;
            Velocity.y = vel.y;
            Velocity.z = vel.z;
        }

        void XM_CALLCONV SetOrientation( FXMVECTOR forward, FXMVECTOR up )
        {
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), forward );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), up );
        }
        void SetOrientation( const XMFLOAT3& forward, const XMFLOAT3& up )
        {
            OrientFront.x = forward.x;  OrientTop.x = up.x;
            OrientFront.y = forward.y;  OrientTop.y = up.y;
            OrientFront.z = forward.z;  OrientTop.z = up.z;
        }

        void XM_CALLCONV SetOrientationFromQuaternion( FXMVECTOR quat )
        {
            XMVECTOR forward = XMVector3Rotate( g_XMIdentityR2, quat );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), forward );

            XMVECTOR up  = XMVector3Rotate( g_XMIdentityR1, quat );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), up );
        }

        void XM_CALLCONV Update( FXMVECTOR newPos, XMVECTOR upDir, float dt )
            // Updates velocity and orientation by tracking changes in position over time...
        {
            if ( dt > 0.f )
            {
                XMVECTOR lastPos = XMLoadFloat3( reinterpret_cast<const XMFLOAT3*>( &Position ) );

                XMVECTOR vDelta = ( newPos - lastPos );
                XMVECTOR v = vDelta / dt;
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Velocity ), v );

                vDelta = XMVector3Normalize( vDelta );
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientFront ), vDelta );

                v = XMVector3Cross( upDir, vDelta );
                v = XMVector3Normalize( v );

                v = XMVector3Cross( vDelta, v );
                v = XMVector3Normalize( v );
                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &OrientTop ), v );

                XMStoreFloat3( reinterpret_cast<XMFLOAT3*>( &Position ), newPos );
            }
        }
    };


    //----------------------------------------------------------------------------------
    class SoundEffectInstance
    {
    public:
        SoundEffectInstance(SoundEffectInstance&& moveFrom);
        SoundEffectInstance& operator= (SoundEffectInstance&& moveFrom);
        virtual ~SoundEffectInstance();

        void Play( bool loop = false );
        void Stop( bool immediate = true );
        void Pause();

        void SetVolume( float volume );
        void SetPitch( float pitch );
        void SetPan( float pan );

        void Apply3D( const AudioListener& listener, const AudioEmitter& emitter );

        bool IsLooped() const;

        enum SoundState
        {
            STOPPED = 0,
            PLAYING,
            PAUSED
        };

        SoundState GetState();

        // Notifications.
        void OnDestroyParent();

    private:
        // Private implementation.
        class Impl;

        std::unique_ptr<Impl> pImpl;

        // Private constructors
        SoundEffectInstance( AudioEngine* engine, SoundEffect* effect, SOUND_EFFECT_INSTANCE_FLAGS flags );
        SoundEffectInstance( AudioEngine* engine, WaveBank* effect, uint32_t index, SOUND_EFFECT_INSTANCE_FLAGS flags );

        friend std::unique_ptr<SoundEffectInstance> SoundEffect::CreateInstance( SOUND_EFFECT_INSTANCE_FLAGS );
        friend std::unique_ptr<SoundEffectInstance> WaveBank::CreateInstance( uint32_t, SOUND_EFFECT_INSTANCE_FLAGS );

        // Prevent copying.
        SoundEffectInstance(SoundEffectInstance const&);
        SoundEffectInstance& operator= (SoundEffectInstance const&);
    };
}

#pragma warning(pop)