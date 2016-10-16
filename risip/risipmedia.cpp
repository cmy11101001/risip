/***********************************************************************************
**    Copyright (C) 2016  Petref Saraci
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You have received a copy of the GNU General Public License
**    along with this program. See LICENSE.GPLv3
**    A copy of the license can be found also here <http://www.gnu.org/licenses/>.
**
************************************************************************************/

#include "risipmedia.h"
#include "risipcall.h"
#include "risipendpoint.h"

#include <QDebug>

RisipMedia::RisipMedia(QObject *parent)
    :QObject(parent)
    ,m_pjsipMediaManager(NULL)
    ,m_callMedia(NULL)
    ,m_localMedia(NULL)
    ,m_playbackDeviceMedia(NULL)
    ,m_activeCall(NULL)
    ,m_sipEndpoint(NULL)
    ,m_keepMediaSettings(true)
{
}

RisipMedia::~RisipMedia()
{
}

RisipCall *RisipMedia::activeCall() const
{
    return m_activeCall;
}

RisipEndpoint *RisipMedia::sipEndpoint() const
{
    return m_sipEndpoint;
}

/**
 * @brief RisipMedia::setSipEndpoint
 * @param endpoint instance of a RisipEndpoint
 *
 * Must set this always in order to initialize the internal pjsip media objects correctly.
 * Not having this property set then no media capabilities will be available when handling a call or
 * simply playing audio.
 *
 * This property is automatically set for each incoming and outgoing call you make.
 */
void RisipMedia::setSipEndpoint(RisipEndpoint *endpoint)
{
    if(m_sipEndpoint != endpoint) {
        m_sipEndpoint = endpoint;
        emit sipEndpointChanged(m_sipEndpoint);

        if(m_sipEndpoint) {
            m_pjsipMediaManager = &m_sipEndpoint->endpointInstance()->audDevManager();
            m_localMedia = &m_pjsipMediaManager->getCaptureDevMedia();
            m_playbackDeviceMedia = &m_pjsipMediaManager->getPlaybackDevMedia();

            //TODO better place where to adjust these media device settings
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_INPUT_VOLUME_SETTING, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_INPUT_ROUTE, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_OUTPUT_ROUTE, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_INPUT_ROUTE, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_INPUT_SIGNAL_METER, 0, true);
//            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_OUTPUT_SIGNAL_METER, 0, true);
            //            pjsua_snd_set_setting(PJMEDIA_AUD_DEV_CAP_EC, 0, true);

//            AudioDevInfoVector audioInfos = m_pjsipMediaManager->enumDev();
//            for(int i=0; i<audioInfos.size(); ++i) {
//                qDebug()<<"Audio device info" << QString::fromStdString(audioInfos[i]->name)
//                       << "Audio caps: " << audioInfos[i]->caps;
//            }
        }
    }
}

/**
 * @brief RisipMedia::setActiveCall
 * @param call
 *
 * Set the parameter call as the active call for this RisipMedia object. The media can start transmiting audio
 * for this call. It sets the active media for that call and therefore call can start answering calls.
 *
 * So when the active_call is answered (@see RisipCall::answer) then media is automatically transimited.
 *
 * internal method - @see RisipCall::initializeMediaHandler and @see RisispCall::setMedia
 */
void RisipMedia::setActiveCall(RisipCall *call)
{
    if(m_activeCall != call) {
        m_activeCall = call;
        emit activeCallChanged(m_activeCall);

        //setting an instance of the risip endpoint object in order to use the singleton instance of pjsip library.
        // see setSipEndpoint for more details
        if(m_activeCall)
            setSipEndpoint(m_activeCall->account()->sipEndPoint());
    }
}

int RisipMedia::speakerVolume() const
{
    if(m_pjsipMediaManager)
        return m_pjsipMediaManager->getOutputVolume();

    return -1;
}

/**
 * @brief RisipMedia::setSpeakerVolume
 * @param volume in percentage 0-100 %
 */
void RisipMedia::setSpeakerVolume(int volume)
{
    if(m_pjsipMediaManager) {
        if(m_pjsipMediaManager->getOutputVolume() != volume) {
            m_pjsipMediaManager->setOutputVolume(volume, m_keepMediaSettings);
            emit speakerVolumeChanged(volume);
        }
    }
}

int RisipMedia::micVolume() const
{
    if(m_pjsipMediaManager)
        return m_pjsipMediaManager->getInputVolume();
}

/**
 * @brief RisipMedia::setMicVolume
 * @param volume in percentage 0-100 %
 */
void RisipMedia::setMicVolume(int volume)
{
    if(m_pjsipMediaManager) {
        if(m_pjsipMediaManager->getInputVolume() != volume) {
            m_pjsipMediaManager->setInputVolume(volume, m_keepMediaSettings);
            emit micVolumeChanged(volume);
        }
    }
}

bool RisipMedia::keepMediaSettings() const
{
    return m_keepMediaSettings;
}

void RisipMedia::setKeepMediaSettings(const bool keep)
{
    if(m_keepMediaSettings != keep) {
        m_keepMediaSettings = keep;
        emit keepMediaSettingsChanged(m_keepMediaSettings);
    }
}

/**
 * @brief RisipMedia::startCallMedia
 *
 * This slot initializes the media engine of Pjsip and starts to transimit multimedia data (this is only audio for now)
 * for an active call. Hence, an active call must have been set prior calling this slot, otherwise the function simply
 * returns.
 *
 * You do not need to ever use or call this function, it is automatically called when you answer a call.
 */
void RisipMedia::startCallMedia()
{
    if(!m_activeCall || !m_sipEndpoint)
        return;

    CallInfo callInfo = m_activeCall->pjsipCall()->getInfo();
    // Iterate all the call medias
    for (int i = 0; i < callInfo.media.size(); ++i) {
        if (callInfo.media[i].type == PJMEDIA_TYPE_AUDIO && m_activeCall->pjsipCall()->getMedia(i)) {
            m_callMedia = (AudioMedia *)m_activeCall->pjsipCall()->getMedia(i);
            break;
        }
    }

    // Connect the call audio media to sound device
    m_localMedia->startTransmit(*m_callMedia);
    m_callMedia->startTransmit(*m_playbackDeviceMedia);

//    m_callMedia->adjustRxLevel()
    m_callMedia->adjustRxLevel(1.0);
    m_callMedia->adjustTxLevel(1.0);
    m_localMedia->adjustRxLevel(1.0);
    m_localMedia->adjustTxLevel(1.0);
}
