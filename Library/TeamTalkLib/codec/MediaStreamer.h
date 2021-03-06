/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#ifndef MEDIASTREAMER_H
#define MEDIASTREAMER_H

#include <ace/ACE.h>
#include <ace/SString.h>
#include <ace/Bound_Ptr.h>

#include <myace/MyACE.h>
#include "MediaUtil.h"

struct MediaFileProp
{
    int audio_channels;
    int audio_samplerate;
    
    int video_width;
    int video_height;
    int video_fps_numerator;
    int video_fps_denominator;

    ACE_UINT32 duration_ms;
    ACE_TString filename;

    MediaFileProp()
    : audio_channels(0), audio_samplerate(0)
    , video_width(0), video_height(0), video_fps_numerator(0)
    , video_fps_denominator(0), duration_ms(0) { }
    MediaFileProp(const ACE_TString& fname)
    : audio_channels(0), audio_samplerate(0)
    , video_width(0), video_height(0), video_fps_numerator(0)
    , video_fps_denominator(0), duration_ms(0), filename(fname) { }
};

struct MediaStreamOutput
{
    bool audio;
    bool video;

    int audio_channels;
    int audio_samplerate;
    int audio_samples;

    MediaStreamOutput()
    : audio(false)
    , video(false)
    , audio_channels(0)
    , audio_samplerate(0)
    , audio_samples(0) {}
};

bool GetMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop);

enum MediaStreamStatus
{
    MEDIASTREAM_STARTED,
    MEDIASTREAM_ERROR,
    MEDIASTREAM_FINISHED
};

class MediaStreamer;

class MediaStreamListener
{
public:
    virtual bool MediaStreamVideoCallback(MediaStreamer* streamer,
                                          media::VideoFrame& video_frame,
                                          ACE_Message_Block* mb_video) = 0;
    //'audio_frame' is a reference to mb_audio->rd_ptr()
    //@return if false 'mb_audio' will be deleted
    virtual bool MediaStreamAudioCallback(MediaStreamer* streamer,
                                          media::AudioFrame& audio_frame,
                                          ACE_Message_Block* mb_audio) = 0;
    virtual void MediaStreamStatusCallback(MediaStreamer* streamer,
                                           const MediaFileProp& mfp,
                                           MediaStreamStatus status) = 0;
};

class MediaStreamer
{
public:
    MediaStreamer(MediaStreamListener* listener) 
        : m_listener(listener), m_stop(false) { }
    virtual ~MediaStreamer() { }
    virtual bool OpenFile(const MediaFileProp& in_prop,
                          const MediaStreamOutput& out_prop) = 0;
    virtual void Close() = 0;

    virtual bool StartStream() = 0;

    const MediaFileProp& GetMediaInput() const { return m_media_in; }
    const MediaStreamOutput& GetMediaOutput() const { return m_media_out; }

protected:
    void Reset();

    MediaFileProp m_media_in;
    MediaStreamOutput m_media_out;
    MediaStreamListener* m_listener;
    bool m_stop;
    
    //return 'true' if it should be called again
    bool ProcessAVQueues(ACE_UINT32 starttime, 
                         int wait_ms, bool flush);

    msg_queue_t m_audio_frames;
    msg_queue_t m_video_frames;

private:
    ACE_UINT32 ProcessAudioFrame(ACE_UINT32 starttime, bool flush);
    ACE_UINT32 ProcessVideoFrame(ACE_UINT32 starttime);
};

typedef ACE_Strong_Bound_Ptr< MediaStreamer, ACE_Null_Mutex > media_streamer_t;

media_streamer_t MakeMediaStreamer(MediaStreamListener* listener);

#endif
